#pragma once
#include <string>
#include <Windows.h>

class Utils
{
public:
    static std::wstring Utf8ToWstring(const std::string& str)
    {
        if (str.empty()) return {};

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);

        wstr.pop_back(); // null terminator 제거 (필요 시)
        return wstr;
    }

    static std::string WstringToUtf8(const std::wstring& wstr)
    {
        if (wstr.empty()) return {};

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string str(size_needed, 0);  // null terminator 포함 크기로 확보
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);

        str.pop_back(); // null terminator 제거 (필요 시)
        return str;
    }
};