#include "string_util.h"

#ifdef WIN32
#include <windows.h>
#else
#include <string.h>
#endif

namespace dy
{
    std::string get_short_name(std::string& fullname)
    {
        if (fullname.empty())
        {
            return fullname;
        }


        std::string::size_type iPos = fullname.find_last_of("/\\");

        if (iPos == std::string::npos)
        {
            return fullname;
        }

        iPos++;
        return fullname.substr(iPos, fullname.length() - iPos);
    }

    char const* strnchr(char const* str, int ch, int which)
    {
        for (int i = 0; i < which; i++)
        {
            if (*(str + i) == ch)
            {
                return str + i;
            }
        }
        return NULL;
    }

    void to_lower(std::string& content)
    {
        for (uint32_t i = 0; i < content.size(); i++)
        {
            if (content[i] >= 'A' && content[i] <= 'Z')
            {
                content[i] += 32;
            }
        }
    }

    static uint8_t hex_to_value(char c)
    {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            return (c - 'a' + 10);
        }

        return 0;
    }

    bool  hex_to_binary(const std::string& hex, std::string& output)
    {
        if (hex.size() % 2)
        {
            return false;
        }

        for (size_t i = 0; i < hex.size(); i += 2)
        {
            uint8_t tmp = 0;

            char hi = hex_to_value(hex[i]);
            char lo = hex_to_value(hex[i + 1]);

            tmp = hi << 4 | lo;

            output.push_back(tmp);
        }
        return true;
    }

    void binary_to_hex(const char* data, uint32_t size, std::string& output)
    {
        for (uint32_t i = 0; i < size; i++)
        {
            char c = data[i];
            if (c >= 0 && c <= 9)
            {
                output.push_back(c + '0');
            }
            else if (c >= 'a' && c <= 'z')
            {
                output.push_back('a' + c);
            }
        }
    }

    void split_string(const char* str, char sep, std::vector<std::string>& strlist)
    {
        strlist.clear();
        const char* start = str;

        const char* p = str;
        std::string tmp;

        while (*p)
        {
            if (*p == sep)
            {
                tmp.assign(start, p - start);
                strlist.push_back(tmp);
                p++;
                start = p;
            }
            else
            {
                p++;
            }
        }

        if (p >= start)
        {
            tmp.assign(start, p - start);
            strlist.push_back(tmp);
        }
    }

    uint64_t StrToUINT64(const char* szBuff)
    {
        uint64_t	ret = 0;
        char* end;
        ret = strtoll(szBuff, &end, 10);
        return ret;
    }

    std::wstring utf8_to_unicode(const std::string& utf8string);

    std::wstring acsi_to_widebyte(const std::string& assic)
    {
#ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &assic[0], (int)assic.size(), nullptr, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &assic[0], (int)assic.size(), &wstrTo[0], size_needed);
        return wstrTo;
#endif
    }

    std::wstring utf8_to_unicode(const std::string& utf8string)
    {
#ifdef _WIN32
        int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION)
        {
            return std::wstring();
        }
        if (widesize == 0)
        {
            return std::wstring();
        }

        std::vector<wchar_t> resultstring(widesize);

        int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

        if (convresult != widesize)
        {
            return std::wstring();
        }

        return std::wstring(&resultstring[0]);
#endif
    }

    std::string utf8_to_acsi(const std::string& utf8string)
    {
        std::wstring strunicode = utf8_to_unicode(utf8string);
        return unicode_to_acsi(strunicode.c_str());
    }

    std::string unicode_to_acsi(const wchar_t* name)
    {
#ifdef _WIN32
        int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, name, -1, NULL, 0, NULL, NULL);
        if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
        {
            return std::string();
        }
        if (asciisize == 0)
        {
            return std::string();
        }
        std::vector<char> resultstring(asciisize);
        int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, name, -1, &resultstring[0], asciisize, NULL, NULL);

        if (convresult != asciisize)
        {
            return std::string();;
        }

        return std::string(&resultstring[0]);
#endif
    }

    std::string unicode_to_utf8(const std::wstring& strUnicode)
    {
#ifdef _WIN32
        int nUtf8Length = WideCharToMultiByte(CP_UTF8,
            0,
            strUnicode.c_str(),
            -1,
            NULL,
            0,
            NULL,
            NULL);

        char* pUtf8 = new char[nUtf8Length + 1];
        memset((void*)pUtf8, 0, sizeof(char) * (nUtf8Length + 1));

        ::WideCharToMultiByte(CP_UTF8,
            0,
            strUnicode.c_str(),
            -1,
            pUtf8,
            nUtf8Length,
            NULL,
            NULL);

        std::string strUtf8;
        strUtf8 = pUtf8;
        delete[] pUtf8;

        return strUtf8;
#endif
    }

    std::string acsi_to_utf8(const std::string& strAssic)
    {
        std::wstring wide = acsi_to_widebyte(strAssic);
        return unicode_to_utf8(wide);
    }
    bool is_str_utf8(const char* str)
    {
        unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节  
        unsigned char chr = *str;
        bool bAllAscii = true;

        for (unsigned int i = 0; str[i] != '\0'; ++i) {
            chr = *(str + i);
            //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx 
            if (nBytes == 0 && (chr & 0x80) != 0) {
                bAllAscii = false;
            }

            if (nBytes == 0) {
                //如果不是ASCII码,应该是多字节符,计算字节数  
                if (chr >= 0x80) {

                    if (chr >= 0xFC && chr <= 0xFD) {
                        nBytes = 6;
                    }
                    else if (chr >= 0xF8) {
                        nBytes = 5;
                    }
                    else if (chr >= 0xF0) {
                        nBytes = 4;
                    }
                    else if (chr >= 0xE0) {
                        nBytes = 3;
                    }
                    else if (chr >= 0xC0) {
                        nBytes = 2;
                    }
                    else {
                        return false;
                    }

                    nBytes--;
                }
            }
            else {
                //多字节符的非首字节,应为 10xxxxxx 
                if ((chr & 0xC0) != 0x80) {
                    return false;
                }
                //减到为零为止
                nBytes--;
            }
        }

        //违返UTF8编码规则 
        if (nBytes != 0) {
            return false;
        }

        if (bAllAscii) { //如果全部都是ASCII, 也是UTF8
            return true;
        }

        return true;
    }

    bool is_str_gbk(const char* str)
    {
        unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个 
        unsigned char chr = *str;
        bool bAllAscii = true; //如果全部都是ASCII,  

        for (unsigned int i = 0; str[i] != '\0'; ++i) {
            chr = *(str + i);
            if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
                bAllAscii = false;
            }

            if (nBytes == 0) {
                if (chr >= 0x80) {
                    if (chr >= 0x81 && chr <= 0xFE) {
                        nBytes = +2;
                    }
                    else {
                        return false;
                    }

                    nBytes--;
                }
            }
            else {
                if (chr < 0x40 || chr>0xFE) {
                    return false;
                }
                nBytes--;
            }//else end
        }

        if (nBytes != 0) {		//违返规则 
            return false;
        }

        if (bAllAscii) { //如果全部都是ASCII, 也是GBK
            return true;
        }

        return true;
    }
}
