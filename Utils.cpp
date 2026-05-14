#define _CRT_SECURE_NO_WARNINGS
#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <random>
#include <functional>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>  // for _mkdir
#endif

// ============================================
// 字符串处理函数实现
// ============================================

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return result;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();

    // 去除开头空白
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    // 去除结尾空白
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return str.substr(start, end - start);
}

bool startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) return false;
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;

    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }

    return result;
}

// ============================================
// 文件操作函数实现
// ============================================

std::string readFileToString(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());

    return content;
}

bool writeStringToFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(content.c_str(), content.size());
    return file.good();
}

bool appendStringToFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    file.write(content.c_str(), content.size());
    return file.good();
}

bool writeBytesToFile(const std::string& filePath, const char* data, size_t size) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(data, size);
    return file.good();
}

size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return 0;
    }

    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    outData.resize(size);
    file.read(outData.data(), size);

    return size;
}

bool fileExists(const std::string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

size_t getFileSize(const std::string& filePath) {
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) != 0) {
        return 0;
    }
    return static_cast<size_t>(buffer.st_size);
}

bool deleteFile(const std::string& filePath) {
    return std::remove(filePath.c_str()) == 0;
}

bool createDirectory(const std::string& dirPath) {
#ifdef _WIN32
    return _mkdir(dirPath.c_str()) == 0;
#else
    return mkdir(dirPath.c_str(), 0777) == 0;
#endif
}

// ============================================
// 时间函数实现
// ============================================

std::string getCurrentTime() {
    auto now = std::time(nullptr);

    struct tm tm_now_buf = {};
    localtime_s(&tm_now_buf, &now);
    std::tm* tm_now = &tm_now_buf;

    std::ostringstream oss;
    oss << std::put_time(tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

long long getCurrentTimestamp() {
    return static_cast<long long>(std::time(nullptr));
}

struct tm parseTimeString(const std::string& timeStr) {
    struct tm tm_result = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm_result, "%Y-%m-%d %H:%M:%S");
    return tm_result;
}

// ============================================
// 其他工具函数实现
// ============================================

int getRandomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

size_t simpleHash(const std::string& str) {
    size_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + static_cast<size_t>(c);
    }
    return hash;
}