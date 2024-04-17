#pragma once

#include <cstdio>
#include <string_view>
#include <vector>
#include "VirtualIO.h"

class FileIO : public VirtualIO
{
private:
    FILE* file{ nullptr };

public:
    FileIO(const char* fileName, const char* mode) noexcept;
    ~FileIO() noexcept override;

    FileIO(FileIO const&) = delete;
    FileIO& operator=(FileIO const&) = delete;

    bool valid() const noexcept { return file != nullptr; }

    int64_t read(void* ptr, int64_t count) noexcept override;

    int64_t write(void* ptr, int64_t count) noexcept;

    int64_t seek(int64_t offset, int whence) noexcept override;

    int64_t tell() const noexcept override;

    int64_t size() const noexcept override;

    static std::vector<unsigned char> LoadData(const char* fileName);

    static bool SaveData(const char* fileName, void* data, int64_t bytesToWrite);

    static bool SaveString(const char* fileName, const std::string_view str);
};
