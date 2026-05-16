#ifndef UTILS_H
#define UTILS_H

// ============================================
// Utils.h - 工具函数头文件
// ============================================
// 功能：提供通用的工具函数，包括：
//   - 字符串处理（分割、转大小写、trim 等）
//   - 文件操作（读写、存在性检查、目录创建等）
//   - 时间函数（获取当前时间、时间戳转换等）
//   - 随机数生成、哈希等
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>
#include <vector>

// ============================================
// 字符串处理函数
// ============================================

// splitString: 字符串分割
// 参数：str - 待分割字符串，delimiter - 分隔符
// 返回：分割后的字符串向量
// 说明：按分隔符拆分字符串，空字符串会被保留
std::vector<std::string> splitString(const std::string& str, char delimiter);

// toLower: 字符串转小写
// 参数：str - 原始字符串
// 返回：小写化后的字符串
// 说明：处理 ASCII 字符，对非字母字符无影响
std::string toLower(const std::string& str);

// toUpper: 字符串转大写
// 参数：str - 原始字符串
// 返回：大写化后的字符串
// 说明：处理 ASCII 字符，对非字母字符无影响
std::string toUpper(const std::string& str);

// trim: 去除字符串首尾空白
// 参数：str - 原始字符串
// 返回：去除首尾空白后的字符串
// 说明：去除空格、制表符、换行符等
std::string trim(const std::string& str);

// startsWith: 检查字符串是否以指定前缀开始
// 参数：str - 原始字符串，prefix - 前缀
// 返回：是返回 true
bool startsWith(const std::string& str, const std::string& prefix);

// endsWith: 检查字符串是否以指定后缀结束
// 参数：str - 原始字符串，suffix - 后缀
// 返回：是返回 true
bool endsWith(const std::string& str, const std::string& suffix);

// replaceAll: 替换字符串中所有匹配项
// 参数：str - 原始字符串，from - 要替换的内容，to - 替换后的内容
// 返回：替换后的字符串
// 说明：替换所有出现的 from 为 to
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

// ============================================
// 文件操作函数
// ============================================

// readFileToString: 读取文件内容到字符串
// 参数：filePath - 文件路径
// 返回：文件内容（空字符串表示读取失败）
// 说明：以二进制模式读取整个文件
std::string readFileToString(const std::string& filePath);

// writeStringToFile: 将字符串写入文件
// 参数：filePath - 文件路径，content - 内容
// 返回：成功返回 true
// 说明：以二进制模式写入，覆盖原文件
bool writeStringToFile(const std::string& filePath, const std::string& content);

// appendStringToFile: 追加字符串到文件
// 参数：filePath - 文件路径，content - 内容
// 返回：成功返回 true
// 说明：以追加模式写入，不覆盖原内容
bool appendStringToFile(const std::string& filePath, const std::string& content);

// writeBytesToFile: 将字节数据写入文件
// 参数：filePath - 文件路径，data - 数据指针，size - 字节数
// 返回：成功返回 true
// 说明：以二进制模式写入
bool writeBytesToFile(const std::string& filePath, const char* data, size_t size);

// readBytesFromFile: 读取文件到字节向量
// 参数：filePath - 文件路径，outData - 输出参数（字节向量）
// 返回：读取的字节数（0 表示失败）
// 说明：以二进制模式读取
size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData);

// fileExists: 检查文件是否存在
// 参数：filePath - 文件路径
// 返回：存在返回 true
bool fileExists(const std::string& filePath);

// getFileSize: 获取文件大小
// 参数：filePath - 文件路径
// 返回：文件字节数（0 表示文件不存在或无法读取）
size_t getFileSize(const std::string& filePath);

// deleteFile: 删除文件
// 参数：filePath - 文件路径
// 返回：成功返回 true
bool deleteFile(const std::string& filePath);

// createDirectory: 创建目录
// 参数：dirPath - 目录路径
// 返回：成功返回 true
// 说明：Windows 使用 _mkdir，Linux 使用 mkdir
bool createDirectory(const std::string& dirPath);

// ============================================
// 时间函数
// ============================================

// getCurrentTime: 获取当前时间字符串
// 返回：格式 "YYYY-MM-DD HH:MM:SS"
// 说明：使用本地时区
std::string getCurrentTime();

// getCurrentTimestamp: 获取当前时间戳
// 返回：自 1970-01-01 以来的秒数
long long getCurrentTimestamp();

// parseTimeString: 解析时间字符串
// 参数：timeStr - 时间字符串（格式：YYYY-MM-DD HH:MM:SS）
// 返回：struct tm 结构
// 说明：解析失败返回零初始化结构
struct tm parseTimeString(const std::string& timeStr);

// ============================================
// 其他工具函数
// ============================================

// getRandomInt: 生成指定范围内的随机整数
// 参数：min - 最小值，max - 最大值
// 返回：范围内的随机整数
// 说明：使用 std::mt19937 随机数引擎
int getRandomInt(int min, int max);

// simpleHash: 简单字符串哈希
// 参数：str - 字符串
// 返回：哈希值（基于 djb2 算法）
// 说明：用于快速哈希计算，碰撞率高于 std::hash
size_t simpleHash(const std::string& str);

#endif // UTILS_H