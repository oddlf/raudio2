#pragma once

#include <string>
#include <string_view>
#include <utility>

// Check file extension
bool IsFileExtension(const char* fileName, const char* ext);

// Get pointer to extension for a filename string (includes the dot: .png)
const char* GetFileExtension(const char* fileName);

std::pair<std::string, std::string> SplitStringIn2(const std::string_view str, char delimiter);

void* LoadExternalLibrary(const char* filePath);

void* GetFunctionAddress(void* handle, const char* functionName);

std::wstring str2wstr(const std::string_view str);
