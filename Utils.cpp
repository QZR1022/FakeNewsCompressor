#define _CRT_SECURE_NO_WARNINGS

// ============================================
// Utils.cpp - 工具函数实现文件
// ============================================
// 功能：提供通用的工具函数，包括：
//   - 字符串处理（分割、转大小写、trim 等）
//   - 文件操作（读写、存在性检查、目录创建等）
//   - 时间函数（获取当前时间、时间戳转换等）
//   - 随机数生成、哈希等
// 作者：QZR1022
// 版本：1.0
// ============================================

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
#include <direct.h>  // for _mkdir on Windows
#endif

// ============================================
// 字符串处理函数实现
// ============================================

// ============================================
// splitString: 字符串分割
// 参数：str - 待分割字符串，delimiter - 分隔符
// 返回：分割后的字符串向量
// 算法：使用 stringstream 按分隔符拆分
// 说明：保留空字符串，如 "a||b" 会产生 ["a", "", "b"]
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

// ============================================
// toLower: 字符串转小写
// 参数：str - 原始字符串
// 返回：小写化后的字符串
// 说明：
//   - 使用 std::tolower 处理每个字符
//   - 需要将 char 转为 unsigned char 避免符号扩展
//   - 只处理 ASCII 字符，对中文无影响
// ============================================
std::string toLower(const std::string& str) {
	std::string result = str;
	for (char& c : result) {
		c = std::tolower(static_cast<unsigned char>(c));
	}
	return result;
}

// ============================================
// toUpper: 字符串转大写
// 参数：str - 原始字符串
// 返回：大写化后的字符串
// 说明：使用 std::toupper 处理每个字符
// ============================================
std::string toUpper(const std::string& str) {
	std::string result = str;
	for (char& c : result) {
		c = std::toupper(static_cast<unsigned char>(c));
	}
	return result;
}

// ============================================
// trim: 去除字符串首尾空白
// 参数：str - 原始字符串
// 返回：去除首尾空白后的字符串
// 说明：
//   - 使用 std::isspace 判断空白字符
//   - 去除空格、制表符、换行符等
//   - 中间空格保留
// ============================================
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

// ============================================
// startsWith: 检查字符串是否以指定前缀开始
// 参数：str - 原始字符串，prefix - 前缀
// 返回：是返回 true
// 说明：前缀为空返回 true
// ============================================
bool startsWith(const std::string& str, const std::string& prefix) {
	if (prefix.length() > str.length()) return false;
	return str.compare(0, prefix.length(), prefix) == 0;
}

// ============================================
// endsWith: 检查字符串是否以指定后缀结束
// 参数：str - 原始字符串，suffix - 后缀
// 返回：是返回 true
// 说明：后缀为空返回 true
// ============================================
bool endsWith(const std::string& str, const std::string& suffix) {
	if (suffix.length() > str.length()) return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// ============================================
// replaceAll: 替换字符串中所有匹配项
// 参数：str - 原始字符串，from - 要替换的内容，to - 替换后的内容
// 返回：替换后的字符串
// 算法：循环查找并替换，直到没有更多匹配
// 说明：
//   - from 为空时不执行替换
//   - to 可以与 from 长度不同
//   - 如果 from 不在 str 中，返回原字符串
// ============================================
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
	std::string result = str;
	size_t pos = 0;

	// from 为空直接返回
	if (from.empty()) return result;

	while ((pos = result.find(from, pos)) != std::string::npos) {
		result.replace(pos, from.length(), to);
		pos += to.length();  // 移动到替换后的位置，避免死循环
	}

	return result;
}

// ============================================
// 文件操作函数实现
// ============================================

// ============================================
// readFileToString: 读取文件内容到字符串
// 参数：filePath - 文件路径
// 返回：文件内容（空字符串表示读取失败）
// 说明：
//   - 以二进制模式读取
//   - 先获取文件大小，预分配字符串空间
//   - 一次性读取全部内容
// ============================================
std::string readFileToString(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return "";  // 文件打开失败
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	// 预分配空间并读取
	std::string content;
	content.resize(fileSize);
	file.read(&content[0], fileSize);

	return content;
}

// ============================================
// writeStringToFile: 将字符串写入文件
// 参数：filePath - 文件路径，content - 内容
// 返回：成功返回 true
// 说明：以二进制模式写入，覆盖原文件
// ============================================
bool writeStringToFile(const std::string& filePath, const std::string& content) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return false;  // 文件打开失败
	}

	file.write(content.c_str(), content.size());
	return file.good();  // 检查写入是否成功
}

// ============================================
// appendStringToFile: 追加字符串到文件
// 参数：filePath - 文件路径，content - 内容
// 返回：成功返回 true
// 说明：以追加模式写入，不覆盖原内容
// ============================================
bool appendStringToFile(const std::string& filePath, const std::string& content) {
	std::ofstream file(filePath, std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		return false;  // 文件打开失败
	}

	file.write(content.c_str(), content.size());
	return file.good();  // 检查写入是否成功
}

// ============================================
// writeBytesToFile: 将字节数据写入文件
// 参数：filePath - 文件路径，data - 数据指针，size - 字节数
// 返回：成功返回 true
// 说明：以二进制模式写入，用于写入压缩数据
// ============================================
bool writeBytesToFile(const std::string& filePath, const char* data, size_t size) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return false;  // 文件打开失败
	}

	file.write(data, size);
	return file.good();  // 检查写入是否成功
}

// ============================================
// readBytesFromFile: 读取文件到字节向量
// 参数：filePath - 文件路径，outData - 输出参数（字节向量）
// 返回：读取的字节数（0 表示失败）
// 说明：以二进制模式读取，用于读取压缩文件
// ============================================
size_t readBytesFromFile(const std::string& filePath, std::vector<char>& outData) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return 0;  // 文件打开失败
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	// 预分配空间并读取
	outData.resize(size);
	file.read(outData.data(), size);

	return file.gcount();  // 返回实际读取的字节数
}

// ============================================
// fileExists: 检查文件是否存在
// 参数：filePath - 文件路径
// 返回：存在返回 true
// 说明：使用 stat 函数获取文件信息
// ============================================
bool fileExists(const std::string& filePath) {
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

// ============================================
// getFileSize: 获取文件大小
// 参数：filePath - 文件路径
// 返回：文件字节数（0 表示文件不存在或无法读取）
// ============================================
size_t getFileSize(const std::string& filePath) {
	struct stat buffer;
	if (stat(filePath.c_str(), &buffer) != 0) {
		return 0;  // 文件不存在或无法读取
	}
	return static_cast<size_t>(buffer.st_size);
}

// ============================================
// deleteFile: 删除文件
// 参数：filePath - 文件路径
// 返回：成功返回 true
// 说明：使用 std::remove 函数
// ============================================
bool deleteFile(const std::string& filePath) {
	return std::remove(filePath.c_str()) == 0;
}

// ============================================
// createDirectory: 创建目录
// 参数：dirPath - 目录路径
// 返回：成功返回 true
// 说明：
//   - Windows 使用 _mkdir（需要完整路径）
//   - Linux 使用 mkdir（权限 0777）
//   - 如果目录已存在也返回 true
// ============================================
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

// ============================================
// getCurrentTime: 获取当前时间字符串
// 返回：格式 "YYYY-MM-DD HH:MM:SS"
// 说明：
//   - 使用 localtime_s 获取本地时间
//   - 使用 put_time 格式化输出
// ============================================
std::string getCurrentTime() {
	auto now = std::time(nullptr);

	// 使用 localtime_s（线程安全版本）
	struct tm tm_now_buf = {};
	localtime_s(&tm_now_buf, &now);
	std::tm* tm_now = &tm_now_buf;

	// 格式化输出
	std::ostringstream oss;
	oss << std::put_time(tm_now, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

// ============================================
// getCurrentTimestamp: 获取当前时间戳
// 返回：自 1970-01-01 以来的秒数
// ============================================
long long getCurrentTimestamp() {
	return static_cast<long long>(std::time(nullptr));
}

// ============================================
// parseTimeString: 解析时间字符串
// 参数：timeStr - 时间字符串（格式：YYYY-MM-DD HH:MM:SS）
// 返回：struct tm 结构
// 说明：使用 get_time 解析，失败返回零初始化结构
// ============================================
struct tm parseTimeString(const std::string& timeStr) {
	struct tm tm_result = {};
	std::istringstream ss(timeStr);
	ss >> std::get_time(&tm_result, "%Y-%m-%d %H:%M:%S");
	return tm_result;
}

// ============================================
// 其他工具函数实现
// ============================================

// ============================================
// getRandomInt: 生成指定范围内的随机整数
// 参数：min - 最小值，max - 最大值
// 返回：范围内的随机整数
// 说明：使用 std::mt19937 随机数引擎（高性能）
// ============================================
int getRandomInt(int min, int max) {
	static std::random_device rd;  // 真随机数种子
	static std::mt19937 gen(rd());  // Mersenne Twister 引擎
	std::uniform_int_distribution<> dis(min, max);
	return dis(gen);
}

// ============================================
// simpleHash: 简单字符串哈希（djb2 算法）
// 参数：str - 字符串
// 返回：哈希值
// 说明：
//   - 经典哈希算法，效率高
//   - 碰撞率高于 std::hash，但足够用于简单用途
//   - 公式：hash = hash * 33 + c（每个字符）
// ============================================
size_t simpleHash(const std::string& str) {
	size_t hash = 5381;  // 初始值（经验值）
	for (char c : str) {
		hash = ((hash << 5) + hash) + static_cast<size_t>(c);
	}
	return hash;
}