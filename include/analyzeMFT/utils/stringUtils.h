#ifndef ANALYZEMFT_STRINGUTILS_H
#define ANALYZEMFT_STRINGUTILS_H

#include <string>
#include <vector>
#include <codecvt>
#include <locale>

class StringUtils {
public:
    static std::string wstringToString(const std::wstring& wstr);
    static std::wstring stringToWstring(const std::string& str);
    static std::string trim(const std::string& str);
    static std::string toLower(const std::string& str);
    static std::string toUpper(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    static std::string escapeForCsv(const std::string& str);
    static std::string sanitizeFilename(const std::string& filename);
    
private:
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
};

#endif