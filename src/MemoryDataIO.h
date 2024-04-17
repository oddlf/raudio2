#pragma once

#include "MemoryIO.h"
#include <vector>

// Memory IO (owning)
class MemoryDataIO : public MemoryIO
{
private:
    std::vector<unsigned char> fileData;

public:
    MemoryDataIO(const char* fileName) noexcept;
};
