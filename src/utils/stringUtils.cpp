#include "stringUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>

std::wstring_convert<std::codecvt_utf8<wchar_t>> StringUtils::converter;

std::string StringUtils::wstringToString(const std::wstring& wstr) {
    try {
        return converter.to_bytes(wstr);
    } catch (const std::exception&) {
        return "";
    }
}

std::wstring StringUtils::stringToWstring(const std::string& str) {
    try {
        return converter.from_bytes(str);
    } catch (const std::exception&) {
        return L"";
    }
}

std::string StringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() && 
           str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() && 
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

std::string StringUtils::escapeForCsv(const std::string& str) {
    if (str.find(',') == std::string::npos && 
        str.find('"') == std::string::npos && 
        str.find('\n') == std::string::npos) {
        return str;
    }
    
    std::string escaped = "\"";
    for (char c : str) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

std::string StringUtils::sanitizeFilename(const std::string& filename) {
    std::string sanitized = filename;
    const std::string invalidChars = "<>:\"/\\|?*";
    
    for (char& c : sanitized) {
        if (invalidChars.find(c) != std::string::npos || c < 32) {
            c = '_';
        }
    }
    
    return sanitized;
}