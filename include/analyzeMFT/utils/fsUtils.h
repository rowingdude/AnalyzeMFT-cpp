#ifndef ANALYZEMFT_FSUTILS_H
#define ANALYZEMFT_FSUTILS_H

#include <string>
#include <vector>
#include <cstdint>

class FileSystemUtils {
public:
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isWritable(const std::string& path);
    static uint64_t getFileSize(const std::string& path);
    static std::string getFileExtension(const std::string& path);
    static std::string getBasename(const std::string& path);
    static std::string getDirname(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    static std::string joinPath(const std::string& path1, const std::string& path2);
    static bool createDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);
    static std::vector<std::string> listDirectory(const std::string& path);
    static bool copyFile(const std::string& source, const std::string& destination);
    static bool moveFile(const std::string& source, const std::string& destination);
    static bool deleteFile(const std::string& path);
    static std::string getTempDirectory();
    static std::string createTempFile(const std::string& prefix = "analyzemft_");
    static uint64_t getAvailableSpace(const std::string& path);
    
private:
    static std::string normalizePath(const std::string& path);
};

#endif