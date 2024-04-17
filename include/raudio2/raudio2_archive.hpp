#pragma once

#include "raudio2_archive.h"
#include "raudio2_virtualio.hpp"

namespace ra
{
    class Archive
    {
    private:
        RAudio2_Archive* archive;

    public:
        Archive(RAudio2_Archive* archive_) noexcept : archive(archive_) {}
        virtual ~Archive() = default;

        explicit operator bool() const noexcept
        {
            return archive != nullptr &&
                   archive->file != nullptr &&
                   archive->file->handle != nullptr &&
                   archive->ctxData != nullptr;
        }

        bool hasValidArchive() const noexcept
        {
            return archive != nullptr;
        }

        bool hasValidFile() const noexcept
        {
            return archive != nullptr &&
                   archive->file != nullptr &&
                   archive->file->handle != nullptr;
        }

        bool hasValidCtxData() const noexcept
        {
            return archive != nullptr &&
                   archive->ctxData != nullptr;
        }

        VirtualIO getFile() const noexcept { return archive->file; }
        void* getCtxData() const noexcept { return archive->ctxData; }

        void setFile(VirtualIO file) noexcept { archive->file = file.getVirtualIO(); }
        void setCtxData(void* ctxData) noexcept { archive->ctxData = ctxData; }
    };
}
