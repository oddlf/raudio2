#include "raudio2_gzip.h"
#include <cstring>
#include <memory>
#include "raudio2/raudio2_archive.hpp"
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include <vector>
#include <zlib.h>

using namespace std::literals;

static int gzipUncompress(Bytef* dest, uLongf* destLen, const Bytef* source, uLong* sourceLen)
{
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len, left;
    Byte buf[1]; /* for detection of incomplete stream when *destLen == 0 */

    len = *sourceLen;
    if (*destLen)
    {
        left = *destLen;
        *destLen = 0;
    }
    else
    {
        left = 1;
        dest = buf;
    }

    stream.next_in = (z_const Bytef*)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit2(&stream, 16 + MAX_WBITS);
    if (err != Z_OK)
        return err;

    stream.next_out = dest;
    stream.avail_out = 0;

    do
    {
        if (stream.avail_out == 0)
        {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0)
        {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    *sourceLen -= len + stream.avail_in;
    if (dest != buf)
        *destLen = stream.total_out;
    else if (stream.total_out && err == Z_BUF_ERROR)
        left = 1;

    inflateEnd(&stream);
    return err == Z_STREAM_END
               ? Z_OK
           : err == Z_NEED_DICT                            ? Z_DATA_ERROR
           : err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR
                                                           : err;
}

static bool gzipUncompress(std::vector<unsigned char>& bytes)
{
    auto uncompressedSize = 0x80000;

    std::vector<unsigned char> uncompressedBytes(uncompressedSize);
    uLongf bytesSize = (uLongf)bytes.size();
    uLongf uncompressedBytesSize = (uLongf)uncompressedBytes.size();
    while (true)
    {
        auto ret = gzipUncompress((Bytef*)uncompressedBytes.data(), &uncompressedBytesSize, (Bytef*)bytes.data(), &bytesSize);
        if (ret == Z_OK)
        {
            uncompressedBytes.resize(uncompressedBytesSize);
            bytes = uncompressedBytes;
            return true;
        }
        else if (ret == Z_BUF_ERROR)
        {
            bytesSize = (uLongf)bytes.size();
            uncompressedBytes.resize(uncompressedBytes.size() * 2);
            uncompressedBytesSize = (uLongf)uncompressedBytes.size();
            continue;
        }
        break;
    }
    return false;
}

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetArchivePlugin(RAudio2_ArchivePlugin* plugin)
{
    return GZIP_MakeArchivePlugin(plugin);
}
#endif

bool GZIP_MakeArchivePlugin(RAudio2_ArchivePlugin* plugin)
{
    plugin->flags = 0;
    plugin->archiveOpen = GZIP_ArchiveOpen;
    plugin->archiveClose = GZIP_ArchiveClose;
    plugin->fileOpen = GZIP_FileOpen;
    plugin->fileRead = GZIP_FileRead;
    plugin->fileSeek = GZIP_FileSeek;
    plugin->fileTell = GZIP_FileTell;
    plugin->fileGetSize = GZIP_FileGetSize;
    plugin->fileClose = GZIP_FileClose;
    plugin->getValue = GZIP_GetValue;

    return true;
}

bool GZIP_ArchiveOpen(RAudio2_Archive* archive_)
{
    ra::Archive archive = archive_;
    if (!archive.hasValidArchive())
        return false;

    archive.setCtxData(nullptr);

    auto file = archive.getFile();
    if (!file)
        return false;

    file.seek(0);

    unsigned char fileBytes[2]{};
    file.read(fileBytes, 2);

    return (fileBytes[0] == 0x1F && fileBytes[1] == 0x8B);
}

bool GZIP_ArchiveClose(RAudio2_Archive* archive)
{
    archive->ctxData = nullptr;
    return true;
}

struct GZIP_File {
    std::vector<unsigned char> fileBytes;
    int64_t currentOffset{};
};

bool GZIP_FileOpen(RAudio2_Archive* archive_, const char* filePath, void** fileCtxOut)
{
    *fileCtxOut = nullptr;

    ra::Archive archive = archive_;
    if (!archive.hasValidArchive())
        return false;

    auto file = archive.getFile();
    if (!file)
        return false;

    file.seek(0);

    auto gzipFile = std::make_unique<GZIP_File>();
    if (!gzipFile)
        return false;

    auto fileSize = file.getSize();
    gzipFile->fileBytes.resize((size_t)fileSize);

    file.read(gzipFile->fileBytes.data(), fileSize);

    if (gzipFile->fileBytes.size() > 1 && gzipFile->fileBytes[0] == 0x1F && gzipFile->fileBytes[1] == 0x8B)
    {
        auto success = gzipUncompress(gzipFile->fileBytes);
        if (success)
            *fileCtxOut = gzipFile.release();
        return success;
    }

    return false;
}

int64_t GZIP_FileRead(void* fileCtx, void* bufferOut, int64_t bytesToRead)
{
    auto gzipFile = (GZIP_File*)fileCtx;
    if (!gzipFile)
        return 0;

    auto dataSize = (int64_t)gzipFile->fileBytes.size();
    int64_t endPosition = gzipFile->currentOffset + bytesToRead;
    int64_t readBytes = endPosition <= dataSize ? bytesToRead : dataSize - gzipFile->currentOffset;

    if (readBytes > 0)
    {
        std::memcpy(bufferOut, gzipFile->fileBytes.data() + gzipFile->currentOffset, (size_t)readBytes);
        gzipFile->currentOffset += readBytes;
    }

    return readBytes;
}

int64_t GZIP_FileSeek(void* fileCtx, int64_t offset, int whence)
{
    auto gzipFile = (GZIP_File*)fileCtx;
    if (!gzipFile)
        return -1;

    auto dataSize = (int64_t)gzipFile->fileBytes.size();
    auto newOffset = offset;
    switch (whence)
    {
    case RAUDIO2_SEEK_CUR:
        newOffset += gzipFile->currentOffset;
        break;
    case RAUDIO2_SEEK_END:
        newOffset += dataSize;
        break;
    default:
        break;
    }

    gzipFile->currentOffset = newOffset < dataSize ? newOffset : dataSize;
    return 0;
}

int64_t GZIP_FileTell(void* fileCtx)
{
    auto gzipFile = (GZIP_File*)fileCtx;
    if (!gzipFile)
        return 0;

    return gzipFile->currentOffset;
}

int64_t GZIP_FileGetSize(void* fileCtx)
{
    auto gzipFile = (GZIP_File*)fileCtx;
    if (!gzipFile)
        return 0;

    return (int64_t)gzipFile->fileBytes.size();
}

bool GZIP_FileClose(void* fileCtx)
{
    auto gzipFile = (GZIP_File*)fileCtx;
    if (!gzipFile)
        return false;

    delete gzipFile;
    return true;
}

bool GZIP_GetValue(void* fileCtx, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("gzip"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 2> extensions{ ".gz", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
        return true;
    }
    default: {
        switch (keyHash)
        {
        case ra::str2int("format"): {
            return ra::MakeValue("GZIP", *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
