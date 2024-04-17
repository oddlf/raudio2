#include "FileIO.h"
#include "raudio2/raudio2_config.h"
#include "Utils.h"

FileIO::FileIO(const char* fileName, const char* mode) noexcept
{
#ifdef RAUDIO2_SYSTEM_WINDOWS
    _wfopen_s(&file, str2wstr(fileName).c_str(), str2wstr(mode).c_str());
#else
    file = fopen(fileName, mode);
#endif
}

FileIO::~FileIO() noexcept
{
    if (!file)
        return;

    fclose(file);
    file = nullptr;
}

int64_t FileIO::read(void* ptr, int64_t count) noexcept
{
    if (!file)
        return 0;

    return (int64_t)fread(ptr, 1, (size_t)count, file);
}

int64_t FileIO::write(void* ptr, int64_t count) noexcept
{
    if (!file)
        return 0;

    return (int64_t)fwrite(ptr, 1, (size_t)count, file);
}

int64_t FileIO::seek(int64_t offset, int whence) noexcept
{
    if (!file)
        return -1;

    return fseek(file, (long)offset, whence);
}

int64_t FileIO::tell() const noexcept
{
    if (!file)
        return 0;

    return (int64_t)ftell(file);
}

int64_t FileIO::size() const noexcept
{
    if (!file)
        return 0;

    auto pos = ftell(file);
    fseek(file, 0L, SEEK_END);
    auto fileSize = (int64_t)ftell(file);
    fseek(file, pos, SEEK_SET);

    if (fileSize <= 0)
        fileSize = 0;

    return fileSize;
}

std::vector<unsigned char> FileIO::LoadData(const char* fileName)
{
    std::vector<unsigned char> data;

    if (fileName != nullptr)
    {
        FileIO file(fileName, "rb");
        if (file.valid())
        {
            auto size = file.size();
            if (size > 0)
            {
                data.resize(size);
                file.read(data.data(), data.size());
            }
        }
    }

    return data;
}

bool FileIO::SaveData(const char* fileName, void* data, int64_t bytesToWrite)
{
    if (fileName != nullptr)
    {
        FileIO file(fileName, "wb");
        if (file.valid())
        {
            file.write(data, bytesToWrite);
            return true;
        }
    }
    return false;
}

bool FileIO::SaveString(const char* fileName, const std::string_view str)
{
    return SaveData(fileName, (void*)str.data(), str.size());
}
