#include "MemoryIO.h"
#include <cstring>

int64_t MemoryIO::read(void* ptr, int64_t count) noexcept
{
    if (!data)
        return 0;

    int64_t endPosition = currentOffset + count;
    int64_t newCount = endPosition <= dataSize ? count : dataSize - currentOffset;

    if (newCount > 0)
    {
        std::memcpy(ptr, data + currentOffset, (size_t)newCount);
        currentOffset += newCount;
    }

    return newCount;
}

int64_t MemoryIO::seek(int64_t offset, int whence) noexcept
{
    if (!data)
        return -1;

    auto newOffset = offset;
    switch (whence)
    {
    case RAUDIO2_SEEK_CUR:
        newOffset += currentOffset;
        break;
    case RAUDIO2_SEEK_END:
        newOffset += dataSize;
        break;
    default:
        break;
    }

    currentOffset = newOffset < dataSize ? newOffset : dataSize;
    return currentOffset;
}

int64_t MemoryIO::tell() const noexcept
{
    if (!data)
        return 0;

    return currentOffset;
}

int64_t MemoryIO::size() const noexcept
{
    if (!data)
        return 0;

    return dataSize;
}
