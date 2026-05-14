#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

// ============================================
// 字符串处理函数
// ============================================

// 字符串分割：按分隔符切割字符串
// 参数：str - 原字符串，delimiter - 分隔符（如 '|' 或 ','）
// 返回：切割后的字符串数组
std::vector<std::string> splitString(const std::string& str, char delimiter);

// 字符串转小写
// 参数：str - 原字符串
// 返回：全小写字符串
std::string toLower(const std::string& str);

// 字符串转大写
// 参数：str - 原字符串
// 返回：全大写字符串
std::string toUpper(const std::string& str);

// 去除字符串首尾空白字符（空格、制表符、换行符、回车符）
// 参数：str - 原字符串
// 返回：去除首尾空白后的字符串
std::string trim(const std::string& str);

// 判断字符串是否以某个前缀开头
// 参数：str - 原字符串，prefix - 前缀
// 返回：是则返回 true，否则 false
bool startsWith(const std::string& str, const std::string& prefix);

// 判断字符串是否以某个后缀结尾
// 参数：str - 原字符串，suffix - 后缀
// 返回：是则返回 true，否则 false
bool endsWith(const std::string& str, const std::string& suffix);

// 替换字符串中的所有子串
// 参数：str - 原字符串，from - 要替换的子串，to - 替换成的子串
// 返回：替换后的字符串
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

// ============================================
// 文件操作函数
// ============================================

// 读取整个文件内容到字符串（文本模式）
// 参数：filePath - 文件路径
// 返回：文件内容，失败返回空字符串
std::string readFileToString(const std::string& filePath);

// 将字符串写入文件（文本模式）
// 参数：filePath - 文件路径，content - 要写入的内容
// 返回：是否写入成功
bool writeStringToFile(const std::string& filePath, const std::string& content);

// 将字符串追加到文件末尾
// 参数：filePath - 文件路径，content - 要追加的内容
// 返回：是否追加成功
bool appendStringToFile(const std::string& filePath, const std::string& content);

// 将字节流写入文件（二进制模式）
// 参数：filePath - 文件路径，data - 字节数组指针，size - 字节数
// 返回：是否写入成功
bool writeBytesToFile(const std::string& filePath, const char* data, size_t size);

// 从文件读取字节流（二进制模式）
// 参数：filePath - 文件路径，outData - 输出字节数组（会被清空并填充）
// 返回：读取的字节数，失败返回 0
size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData);

// 检查文件是否存在
// 参数：filePath - 文件路径
// 返回：存在返回 true，否则 false
bool fileExists(const std::string& filePath);

// 获取文件大小（字节）
// 参数：filePath - 文件路径
// 返回：文件大小，失败返回 0
size_t getFileSize(const std::string& filePath);

// 删除文件
// 参数：filePath - 文件路径
// 返回：成功返回 true
bool deleteFile(const std::string& filePath);

// 创建目录（如果不存在）
// 参数：dirPath - 目录路径
// 返回：成功返回 true（目录已存在或创建成功）
bool createDirectory(const std::string& dirPath);

// ============================================
// 时间函数
// ============================================

// 获取当前时间字符串，格式 "YYYY-MM-DD HH:MM:SS"
// 返回：格式化的时间字符串
std::string getCurrentTime();

// 获取当前时间戳（秒数，自 1970-01-01 起）
// 返回：时间戳
long long getCurrentTimestamp();

// 将时间字符串解析为 tm 结构体
// 参数：timeStr - 格式 "YYYY-MM-DD HH:MM:SS"
// 返回：tm 结构体
struct tm parseTimeString(const std::string& timeStr);

// ============================================
// 其他工具函数
// ============================================

// 生成随机数（范围 [min, max]）
// 参数：min - 最小值，max - 最大值
// 返回：随机整数
int getRandomInt(int min, int max);

// 计算字符串的简单哈希（用于快速比较）
// 参数：str - 输入字符串
// 返回：哈希值
size_t simpleHash(const std::string& str);

#endif // UTILS_H