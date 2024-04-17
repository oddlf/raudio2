#pragma once

#include <cstdint>
#include <memory>
#include "raudio2/raudio2_inputplugin.hpp"

class VirtualIO
{
public:
    virtual ~VirtualIO() = default;

    virtual int64_t read(void* buffer, int64_t count) = 0;

    virtual int64_t seek(int64_t offset, int whence) = 0;

    virtual int64_t tell() const = 0;

    virtual int64_t size() const = 0;
};

class VirtualIOWrapper
{
private:
    std::unique_ptr<VirtualIO> file;
    RAudio2_VirtualIO virtualIO;

public:
    VirtualIOWrapper();
    VirtualIOWrapper(std::unique_ptr<VirtualIO>&& file_);

    explicit operator bool() const noexcept { return file != nullptr; }

    auto& GetVirtualIO() noexcept { return virtualIO; }
    auto& GetVirtualIO() const noexcept { return virtualIO; }

    void setFile(std::unique_ptr<VirtualIO>&& file_);

    int64_t read(void* buffer, int64_t count) noexcept;

    int64_t seek(int64_t offset, int whence) noexcept;

    int64_t tell() const noexcept;

    int64_t size() const noexcept;
};
