#include "Utils.h"
#include <codecvt>
#include <cstring>
#include <iostream>
#include <locale>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(__unix__)
#include <dlfcn.h>
#endif

bool IsFileExtension(const char* fileName, const char* ext)
{
    bool result = false;
    const char* fileExt;

    if ((fileExt = strrchr(fileName, '.')) != nullptr)
    {
        if (strcmp(fileExt, ext) == 0)
            result = true;
    }

    return result;
}

const char* GetFileExtension(const char* fileName)
{
    const char* dot = strrchr(fileName, '.');

    if (!dot || dot == fileName)
        return nullptr;

    return dot;
}

std::pair<std::string, std::string> SplitStringIn2(const std::string_view str, char delimiter)
{
    auto pos = str.find(delimiter, 0);
    if (pos != std::string::npos)
    {
        return { std::string(str.substr(0, pos)), std::string(str.substr(pos + 1, str.size() - pos)) };
    }
    return { std::string(str), "" };
}

void* LoadExternalLibrary(const char* filePath)
{
#if defined(_WIN32)
    return LoadLibraryA(filePath);
#elif defined(__unix__)
    return dlopen(filePath, RTLD_LAZY | RTLD_LOCAL);
#else
    return nullptr;
#endif
}

void* GetFunctionAddress(void* handle, const char* functionName)
{
#if defined(_WIN32)
    return (void*)GetProcAddress((HMODULE)handle, functionName);
#elif defined(__unix__)
    return dlsym(handle, functionName);
#else
    return nullptr;
#endif
}

std::wstring str2wstr(const std::string_view str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str.data(), str.data() + str.size());
}
