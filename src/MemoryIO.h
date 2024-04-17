#pragma once

#include "VirtualIO.h"

// Memory IO (non owning)
class MemoryIO : public VirtualIO
{
protected:
    const unsigned char* data{ nullptr };
    int64_t dataSize{ 0 };
    int64_t currentOffset{ 0 };

public:
    MemoryIO() noexcept = default;
    MemoryIO(const void* data_, int64_t sizeInBytes) noexcept
        : data((const unsigned char*)data_), dataSize(sizeInBytes){};

    ~MemoryIO() noexcept override = default;

    MemoryIO(MemoryIO const&) = delete;
    MemoryIO& operator=(MemoryIO const&) = delete;

    int64_t read(void* ptr, int64_t count) noexcept override;

    int64_t seek(int64_t offset, int whence) noexcept override;

    int64_t tell() const noexcept override;

    int64_t size() const noexcept override;
};
