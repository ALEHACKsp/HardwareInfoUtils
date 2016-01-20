#pragma once

#include <string>
#include <algorithm>

using namespace std;

void toLower(string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
}

std::string wstringTostring(const std::wstring &wstrSrc)
{
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const wchar_t *_Source = wstrSrc.c_str();
    size_t _Dsize = 2 * wstrSrc.size() + 1;
    char *_Dest = new char[_Dsize];
    memset(_Dest, 0, _Dsize);
    wcstombs(_Dest, _Source, _Dsize);
    std::string result = _Dest;
    delete[]_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::wstring stringTowstring(const std::string &s)
{
    int wslen;
    int slen = (int)s.length();
    wslen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slen, 0, 0);
    std::wstring r(wslen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slen, &r[0], wslen);
    return r;
}

bool startsWith(string & str, string & substr, bool ignoreCase = false)
{
    if (ignoreCase)
    {
        string str1 = str;
        string str2 = substr;
        toLower(str1);
        toLower(str2);

        return startsWith(str1, str2);
    }

    return str.find(substr) == 0;
}

bool endsWith(string & str, string & substr, bool ignoreCase = false)
{
    if (ignoreCase)
    {
        string str1 = str;
        string str2 = substr;
        toLower(str1);
        toLower(str2);
        return endsWith(str1, str2);
    }

    return str.rfind(substr) == (str.length() - substr.length());
}
