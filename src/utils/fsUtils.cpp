#include "fsUtils.h"
#include <sys/stat.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#define PATH_SEPARATOR '\\'
#define mkdir(path, mode) _mkdir(path)
#define access _access
#define F_OK 0
#define R_OK 4
#define W_OK 2
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/statvfs.h>
#define PATH_SEPARATOR '/'
#endif

bool FileSystemUtils::fileExists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool FileSystemUtils::isDirectory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

bool FileSystemUtils::isReadable(const std::string& path) {
    return access(path.c_str(), R_OK) == 0;
}

bool FileSystemUtils::isWritable(const std::string& path) {
    return access(path.c_str(), W_OK) == 0;
}

uint64_t FileSystemUtils::getFileSize(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return 0;
    }
    return static_cast<uint64_t>(statbuf.st_size);
}

std::string FileSystemUtils::getFileExtension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos || pos == path.length() - 1) {
        return "";
    }
    return path.substr(pos + 1);
}

std::string FileSystemUtils::getBasename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

std::string FileSystemUtils::getDirname(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return ".";
    }
    if (pos == 0) {
        return "/";
    }
    return path.substr(0, pos);
}

std::string FileSystemUtils::getAbsolutePath(const std::string& path) {
#ifdef _WIN32
    char resolved[MAX_PATH];
    if (_fullpath(resolved, path.c_str(), MAX_PATH) != nullptr) {
        return std::string(resolved);
    }
#else
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved) != nullptr) {
        return std::string(resolved);
    }
#endif
    return path;
}

std::string FileSystemUtils::joinPath(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    std::string result = path1;
    if (result.back() != PATH_SEPARATOR && path2.front() != PATH_SEPARATOR) {
        result += PATH_SEPARATOR;
    } else if (result.back() == PATH_SEPARATOR && path2.front() == PATH_SEPARATOR) {
        result.pop_back();
    }
    result += path2;
    return result;
}

bool FileSystemUtils::createDirectory(const std::string& path) {
    return mkdir(path.c_str(), 0755) == 0 || isDirectory(path);
}

bool FileSystemUtils::createDirectories(const std::string& path) {
    if (path.empty() || isDirectory(path)) {
        return true;
    }
    
    std::string parent = getDirname(path);
    if (!createDirectories(parent)) {
        return false;
    }
    
    return createDirectory(path);
}

std::vector<std::string> FileSystemUtils::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    std::string searchPath = joinPath(path, "*");
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findData.cFileName;
            if (filename != "." && filename != "..") {
                files.push_back(filename);
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(path.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename != "." && filename != "..") {
                files.push_back(filename);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

bool FileSystemUtils::copyFile(const std::string& source, const std::string& destination) {
    std::ifstream src(source, std::ios::binary);
    if (!src.is_open()) {
        return false;
    }
    
    std::ofstream dst(destination, std::ios::binary);
    if (!dst.is_open()) {
        return false;
    }
    
    dst << src.rdbuf();
    return src.good() && dst.good();
}

bool FileSystemUtils::moveFile(const std::string& source, const std::string& destination) {
    if (rename(source.c_str(), destination.c_str()) == 0) {
        return true;
    }
    
    if (copyFile(source, destination)) {
        return deleteFile(source);
}
   return false;
}

bool FileSystemUtils::deleteFile(const std::string& path) {
   return remove(path.c_str()) == 0;
}

std::string FileSystemUtils::getTempDirectory() {
#ifdef _WIN32
   char tempPath[MAX_PATH];
   DWORD result = GetTempPath(MAX_PATH, tempPath);
   if (result > 0 && result < MAX_PATH) {
       return std::string(tempPath);
   }
   return "C:\\temp\\";
#else
   const char* tmpDir = getenv("TMPDIR");
   if (tmpDir) return std::string(tmpDir);
   
   tmpDir = getenv("TMP");
   if (tmpDir) return std::string(tmpDir);
   
   tmpDir = getenv("TEMP");
   if (tmpDir) return std::string(tmpDir);
   
   return "/tmp/";
#endif
}

std::string FileSystemUtils::createTempFile(const std::string& prefix) {
   std::string tempDir = getTempDirectory();
   std::string tempFile;
   
   for (int i = 0; i < 1000; ++i) {
       std::ostringstream oss;
       oss << prefix << "_" << std::time(nullptr) << "_" << i;
       tempFile = joinPath(tempDir, oss.str());
       
       if (!fileExists(tempFile)) {
           std::ofstream file(tempFile);
           if (file.is_open()) {
               file.close();
               return tempFile;
           }
       }
   }
   
   return "";
}

uint64_t FileSystemUtils::getAvailableSpace(const std::string& path) {
#ifdef _WIN32
   ULARGE_INTEGER freeBytes;
   if (GetDiskFreeSpaceEx(path.c_str(), &freeBytes, nullptr, nullptr)) {
       return freeBytes.QuadPart;
   }
#else
   struct statvfs statInfo;
   if (statvfs(path.c_str(), &statInfo) == 0) {
       return static_cast<uint64_t>(statInfo.f_bavail) * statInfo.f_frsize;
   }
#endif
   return 0;
}

std::string FileSystemUtils::normalizePath(const std::string& path) {
   std::string normalized = path;
   std::replace(normalized.begin(), normalized.end(), '\\', PATH_SEPARATOR);
   std::replace(normalized.begin(), normalized.end(), '/', PATH_SEPARATOR);
   return normalized;
}