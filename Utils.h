#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <ctime>

// 字符串分割：按分隔符切割字符串
// 参数：str - 原字符串，delimiter - 分隔符（如 "," 或 "|"）
// 返回：切割后的字符串数组
std::vector<std::string> splitString(const std::string& str, char delimiter);

// 字符串转小写
// 参数：str - 原字符串
// 返回：全小写字符串
std::string toLower(const std::string& str);

// 去除字符串首尾空白字符（空格、制表符、换行符）
// 参数：str - 原字符串
// 返回：去除首尾空白后的字符串
std::string trim(const std::string& str);

// 读取整个文件内容到字符串
// 参数：filePath - 文件路径
// 返回：文件内容，失败返回空字符串
std::string readFileToString(const std::string& filePath);

// 将字符串写入文件（二进制模式）
// 参数：filePath - 文件路径，content - 要写入的内容
// 返回：是否写入成功
bool writeStringToFile(const std::string& filePath, const std::string& content);

// 将字节流写入文件
// 参数：filePath - 文件路径，data - 字节数组，size - 字节数
// 返回：是否写入成功
bool writeBytesToFile(const std::string& filePath, const char* data, size_t size);

// 从文件读取字节流
// 参数：filePath - 文件路径，outData - 输出字节数组
// 返回：读取的字节数，失败返回0
size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData);

// 获取当前时间字符串，格式 "YYYY-MM-DD HH:MM:SS"
// 返回：格式化的时间字符串
std::string getCurrentTime();

// 检查文件是否存在
// 参数：filePath - 文件路径
// 返回：存在返回true，否则false
bool fileExists(const std::string& filePath);

// 创建目录（如果不存在）
// 参数：dirPath - 目录路径
// 返回：成功返回true
bool createDirectoryIfNotExists(const std::string& dirPath);

#endif // UTILS_H


