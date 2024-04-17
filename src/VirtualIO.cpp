#include "VirtualIO.h"

static int64_t VirtualIO_Read(void* handle, void* buffer, int64_t count)
{
    auto file = (VirtualIO*)handle;
    if (!file)
        return 0;

    return file->read(buffer, count);
}

static int64_t VirtualIO_Seek(void* handle, int64_t offset, int whence)
{
    auto file = (VirtualIO*)handle;
    if (!file)
        return -1;

    return file->seek(offset, whence);
}

static int64_t VirtualIO_Tell(void* handle)
{
    auto file = (VirtualIO*)handle;
    if (!file)
        return 0;

    return file->tell();
}

static int64_t VirtualIO_Size(void* handle)
{
    auto file = (VirtualIO*)handle;
    if (!file)
        return 0;

    return file->size();
}

VirtualIOWrapper::VirtualIOWrapper()
{
    virtualIO.handle = nullptr;
    virtualIO.read = VirtualIO_Read;
    virtualIO.write = nullptr;
    virtualIO.seek = VirtualIO_Seek;
    virtualIO.tell = VirtualIO_Tell;
    virtualIO.getSize = VirtualIO_Size;
}

VirtualIOWrapper::VirtualIOWrapper(std::unique_ptr<VirtualIO>&& file_) : VirtualIOWrapper()
{
    setFile(std::move(file_));
}

void VirtualIOWrapper::setFile(std::unique_ptr<VirtualIO>&& file_)
{
    file = std::move(file_);
    if (file)
    {
        virtualIO.handle = file.get();
    }
}

int64_t VirtualIOWrapper::read(void* buffer, int64_t count) noexcept
{
    return virtualIO.read(virtualIO.handle, buffer, count);
}

int64_t VirtualIOWrapper::seek(int64_t offset, int whence) noexcept
{
    return virtualIO.seek(virtualIO.handle, offset, whence);
}

int64_t VirtualIOWrapper::tell() const noexcept
{
    return virtualIO.tell(virtualIO.handle);
}

int64_t VirtualIOWrapper::size() const noexcept
{
    return virtualIO.getSize(virtualIO.handle);
}
