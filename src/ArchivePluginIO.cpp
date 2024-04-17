#include "ArchivePluginIO.h"

int64_t ArchivePluginIO::read(void* ptr, int64_t count) noexcept
{
    if (!plugin || !fileCtx)
        return 0;

    return plugin.fileRead(fileCtx, ptr, count);
}

int64_t ArchivePluginIO::write(void* ptr, int64_t count) noexcept
{
    return 0;
}

int64_t ArchivePluginIO::seek(int64_t offset, int whence) noexcept
{
    if (!plugin || !fileCtx)
        return -1;

    return plugin.fileSeek(fileCtx, offset, whence);
}

int64_t ArchivePluginIO::tell() const noexcept
{
    if (!plugin || !fileCtx)
        return 0;

    return plugin.fileTell(fileCtx);
}

int64_t ArchivePluginIO::size() const noexcept
{
    if (!plugin || !fileCtx)
        return 0;

    return plugin.fileGetSize(fileCtx);
}
