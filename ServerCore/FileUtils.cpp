#include "pch.h"
#include "FileUtils.h"
#include <fstream>
#include <filesystem>

/*------------------------
         FileUtils
-------------------------*/

namespace fs = std::filesystem;

Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
    Vector<BYTE> ret;
    wstring wPath = path;
    fs::path filePath(wPath);

    const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath));
    ret.resize(fileSize);

    basic_ifstream<BYTE> inputStream{ filePath };
    inputStream.read(&ret[0], fileSize);

    return ret;
}

WString FileUtils::Convert(WString str)
{
    const int32 srcLen = static_cast<int32>(str.size());

    WString ret;
    if (srcLen == 0)
        return ret;

    const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
    ret.resize(retLen);
    ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

    return ret;
}

