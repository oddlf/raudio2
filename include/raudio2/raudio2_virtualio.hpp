#pragma once

#include "raudio2_virtualio.h"

namespace ra
{
    class VirtualIO
    {
    private:
        RAudio2_VirtualIO* io;

    public:
        VirtualIO(RAudio2_VirtualIO* io_) noexcept : io(io_) {}
        virtual ~VirtualIO() = default;

        explicit operator bool() const noexcept { return io != nullptr && io->handle != nullptr; }

        RAudio2_VirtualIO* getVirtualIO() const noexcept { return io; }

        int64_t read(void* bufferOut, int64_t count) noexcept { return io->read(io->handle, bufferOut, count); }

        int64_t seek(int64_t offset, int whence = RAUDIO2_SEEK_SET) noexcept { return io->seek(io->handle, offset, whence); }

        int64_t tell() const noexcept { return io->tell(io->handle); }

        int64_t getSize() const noexcept { return io->getSize(io->handle); }
    };
}
