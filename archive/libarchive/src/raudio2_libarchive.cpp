#include "raudio2_libarchive.h"
#include <archive.h>
#include <archive_entry.h>
#include <cstring>
#include <memory>
#include "raudio2/raudio2_archive.hpp"
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include <vector>

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetArchivePlugin(RAudio2_ArchivePlugin* plugin)
{
    return LIBARCHIVE_MakeArchivePlugin(plugin);
}
#endif

bool LIBARCHIVE_MakeArchivePlugin(RAudio2_ArchivePlugin* plugin)
{
    plugin->flags = 0;
    plugin->archiveOpen = LIBARCHIVE_ArchiveOpen;
    plugin->archiveClose = LIBARCHIVE_ArchiveClose;
    plugin->fileOpen = LIBARCHIVE_FileOpen;
    plugin->fileRead = LIBARCHIVE_FileRead;
    plugin->fileSeek = LIBARCHIVE_FileSeek;
    plugin->fileTell = LIBARCHIVE_FileTell;
    plugin->fileGetSize = LIBARCHIVE_FileGetSize;
    plugin->fileClose = LIBARCHIVE_FileClose;
    plugin->getValue = LIBARCHIVE_GetValue;

    return true;
}

struct LIBARCHIVE_Archive {
    archive* archive_{};
    archive_entry* firstEntry{};
    std::vector<unsigned char> buffer;
    int64_t currentOffset{};
};

static int LIBARCHIVE_OnNull(struct archive* archive_, void* userData)
{
    return ARCHIVE_OK;
}

// Returns pointer and size of next block of data from archive.
static la_ssize_t LIBARCHIVE_OnRead(struct archive* archive_, void* archiveCtx, const void** bufferOut)
{
    *bufferOut = nullptr;

    auto raudioArchive = (RAudio2_Archive*)archiveCtx;
    if (!raudioArchive)
        return 0;

    auto libArchive = (LIBARCHIVE_Archive*)raudioArchive->ctxData;
    if (!libArchive)
        return 0;

    if (libArchive->buffer.size() < 0x80000)
    {
        libArchive->buffer.resize(0x80000);
    }
    *bufferOut = libArchive->buffer.data();

    ra::VirtualIO file = raudioArchive->file;

    return file.read(libArchive->buffer.data(), (int64_t)libArchive->buffer.size());
}

// Seeks to specified location in the file and returns the position.
// Whence values are SEEK_SET, SEEK_CUR, SEEK_END from stdio.h.
// Return ARCHIVE_FATAL if the seek fails for any reason.
static la_int64_t LIBARCHIVE_OnSeek(struct archive* archive_, void* archiveCtx, la_int64_t offset, int whence)
{
    auto raudioArchive = (RAudio2_Archive*)archiveCtx;
    if (!raudioArchive)
        return ARCHIVE_FATAL;

    ra::VirtualIO file(raudioArchive->file);
    if (!file)
        return ARCHIVE_FATAL;

    if (file.seek(offset, whence) == 0)
        return file.tell();

    return ARCHIVE_FATAL;
}

// Skips at most request bytes from archive and returns the skipped amount.
// This may skip fewer bytes than requested; it may even skip zero bytes.
// If you do skip fewer bytes than requested, libarchive will invoke your
// read callback and discard data as necessary to make up the full skip.
static la_int64_t LIBARCHIVE_OnSkip(struct archive* archive_, void* archiveCtx, la_int64_t skipBytes)
{
    auto raudioArchive = (RAudio2_Archive*)archiveCtx;
    if (!raudioArchive)
        return 0;

    ra::VirtualIO file(raudioArchive->file);
    if (!file)
        return 0;

    auto currPos = file.tell();
    file.seek(skipBytes, RAUDIO2_SEEK_CUR);
    return file.tell() - currPos;
}

bool LIBARCHIVE_ArchiveOpen(RAudio2_Archive* archive_)
{
    ra::Archive raudioArchive = archive_;
    if (!raudioArchive.hasValidArchive())
        return false;

    raudioArchive.setCtxData(nullptr);

    auto file = raudioArchive.getFile();
    if (!file)
        return false;

    file.seek(0);

    auto archiveStruct = std::make_unique<LIBARCHIVE_Archive>();
    if (!archiveStruct)
        return false;

    auto archiveCtx = archive_read_new();
    archiveStruct->archive_ = archiveCtx;
    raudioArchive.setCtxData(archiveStruct.get());

    archive_read_support_filter_all(archiveCtx);
    archive_read_support_format_all(archiveCtx);
    archive_read_support_format_raw(archiveCtx);

    archive_read_set_callback_data(archiveCtx, archive_);
    archive_read_set_open_callback(archiveCtx, LIBARCHIVE_OnNull);
    archive_read_set_read_callback(archiveCtx, LIBARCHIVE_OnRead);
    archive_read_set_seek_callback(archiveCtx, LIBARCHIVE_OnSeek);
    archive_read_set_skip_callback(archiveCtx, LIBARCHIVE_OnSkip);
    archive_read_set_close_callback(archiveCtx, LIBARCHIVE_OnNull);

    auto ret = archive_read_open1(archiveCtx);
    if (ret != ARCHIVE_OK)
        return false;

    ret = archive_read_next_header(archiveCtx, &archiveStruct->firstEntry);
    if (ret != ARCHIVE_OK)
    {
        archive_read_free(archiveCtx);
        return false;
    }

    auto archiveFormat = archive_format(archiveCtx);
    auto archiveFilter = archive_filter_code(archiveCtx, 0);
    if (archiveFormat == ARCHIVE_FORMAT_RAW && archiveFilter == ARCHIVE_FILTER_NONE)
    {
        archive_read_free(archiveCtx);
        return false;
    }

    raudioArchive.setCtxData(archiveStruct.release());

    return true;
}

bool LIBARCHIVE_ArchiveClose(RAudio2_Archive* archive_)
{
    ra::Archive raudioArchive = archive_;
    if (!raudioArchive.hasValidArchive())
        return false;

    auto libArchive = (LIBARCHIVE_Archive*)raudioArchive.getCtxData();
    if (!libArchive)
        return false;

    archive_read_free(libArchive->archive_);
    delete libArchive;
    raudioArchive.setCtxData(nullptr);
    return true;
}

struct LIBARCHIVE_File {
    std::vector<unsigned char> fileBytes;
    int64_t currentOffset{};
};

bool LIBARCHIVE_FileOpen(RAudio2_Archive* archive_, const char* filePath, void** fileCtxOut)
{
    *fileCtxOut = nullptr;

    ra::Archive raudioArchive = archive_;
    if (!raudioArchive.hasValidArchive())
        return false;

    auto file = raudioArchive.getFile();
    if (!file)
        return false;

    auto libArchiveCtx = (LIBARCHIVE_Archive*)raudioArchive.getCtxData();
    if (!libArchiveCtx)
        return false;

    auto libArchiveFile = std::make_unique<LIBARCHIVE_File>();
    if (!libArchiveFile)
        return false;

    auto entry = libArchiveCtx->firstEntry;

    while (true)
    {
        bool success = filePath == nullptr || filePath[0] == '\0';
        if (!success)
        {
            std::string_view pathName(archive_entry_pathname_utf8(entry));
            success = pathName == filePath || pathName == "data"sv;
        }
        if (success)
        {
            const void* buff{};
            size_t size{};
            int64_t offset{};
            int r = 0;
            while ((r = archive_read_data_block(libArchiveCtx->archive_, &buff, &size, &offset)) == ARCHIVE_OK)
            {
                libArchiveFile->fileBytes.resize(libArchiveFile->fileBytes.size() + size);
                std::memcpy(libArchiveFile->fileBytes.data() + offset, buff, size);
            }

            *fileCtxOut = libArchiveFile.release();
            return true;
        }

        auto ret = archive_read_next_header2(libArchiveCtx->archive_, entry);
        if (ret == ARCHIVE_EOF)
            return false;
        if (ret != ARCHIVE_OK)
            return false;
    }

    return false;
}

int64_t LIBARCHIVE_FileRead(void* fileCtx, void* bufferOut, int64_t bytesToRead)
{
    auto libArchiveFile = (LIBARCHIVE_File*)fileCtx;
    if (!libArchiveFile)
        return 0;

    auto dataSize = (int64_t)libArchiveFile->fileBytes.size();
    int64_t endPosition = libArchiveFile->currentOffset + bytesToRead;
    int64_t readBytes = endPosition <= dataSize ? bytesToRead : dataSize - libArchiveFile->currentOffset;

    if (readBytes > 0)
    {
        std::memcpy(bufferOut, libArchiveFile->fileBytes.data() + libArchiveFile->currentOffset, (size_t)readBytes);
        libArchiveFile->currentOffset += readBytes;
    }

    return readBytes;
}

int64_t LIBARCHIVE_FileSeek(void* fileCtx, int64_t offset, int whence)
{
    auto libArchiveFile = (LIBARCHIVE_File*)fileCtx;
    if (!libArchiveFile)
        return -1;

    auto dataSize = (int64_t)libArchiveFile->fileBytes.size();
    auto newOffset = offset;
    switch (whence)
    {
    case RAUDIO2_SEEK_CUR:
        newOffset += libArchiveFile->currentOffset;
        break;
    case RAUDIO2_SEEK_END:
        newOffset += dataSize;
        break;
    default:
        break;
    }

    libArchiveFile->currentOffset = newOffset < dataSize ? newOffset : dataSize;
    return 0;
}

int64_t LIBARCHIVE_FileTell(void* fileCtx)
{
    auto libArchiveFile = (LIBARCHIVE_File*)fileCtx;
    if (!libArchiveFile)
        return 0;

    return libArchiveFile->currentOffset;
}

int64_t LIBARCHIVE_FileGetSize(void* fileCtx)
{
    auto libArchiveFile = (LIBARCHIVE_File*)fileCtx;
    if (!libArchiveFile)
        return 0;

    return (int64_t)libArchiveFile->fileBytes.size();
}

bool LIBARCHIVE_FileClose(void* fileCtx)
{
    auto libArchiveFile = (LIBARCHIVE_File*)fileCtx;
    if (!libArchiveFile)
        return false;

    delete libArchiveFile;
    return true;
}

bool LIBARCHIVE_GetValue(void* fileCtx, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("libarchive"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 6> extensions{ ".7z", ".gz", ".rar", ".tar", ".zip", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
        return true;
    }
    default: {
        break;
    }
    }
    *valueOut = {};
    return false;
}
