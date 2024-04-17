#pragma once

#include "raudio2/raudio2_archiveplugin.hpp"
#include "VirtualIO.h"

class ArchivePluginIO : public VirtualIO
{
private:
    ra::ArchivePlugin plugin;
    void* fileCtx{};

public:
    ArchivePluginIO(ra::ArchivePlugin plugin_, void* fileCtx_) noexcept : plugin(plugin_), fileCtx(fileCtx_){};

    ArchivePluginIO(ArchivePluginIO const&) = delete;
    ArchivePluginIO& operator=(ArchivePluginIO const&) = delete;

    bool valid() const noexcept { return (bool)plugin; }

    int64_t read(void* ptr, int64_t count) noexcept override;

    int64_t write(void* ptr, int64_t count) noexcept;

    int64_t seek(int64_t offset, int whence) noexcept override;

    int64_t tell() const noexcept override;

    int64_t size() const noexcept override;
};
