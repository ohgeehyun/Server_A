#pragma once

/*------------------------
         FileUtils
-------------------------*/
class FileUtils
{
public:
    static  Vector<BYTE>  ReadFile(const WCHAR* path);
    static  WString        Convert(WString str);
};

