#include "MemoryDataIO.h"
#include "FileIO.h"

MemoryDataIO::MemoryDataIO(const char* fileName) noexcept
{
    fileData = FileIO::LoadData(fileName);
    if (!fileData.empty())
    {
        data = fileData.data();
        dataSize = (int64_t)fileData.size();
    }
}
