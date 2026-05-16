#define _CRT_SECURE_NO_WARNINGS

// ============================================
// LZ77.cpp - LZ77 压缩算法实现文件
// ============================================
// 功能：实现 LZ77 滑动窗口压缩/解压算法
// 说明：每个 token 占 4 字节：[offset(2B)][length(1B)][literal(1B)]
//       offset=0 表示无匹配，只有 literal
// 作者：QZR1022
// 版本：1.0
// ============================================

#include "LZ77.h"
#include "Utils.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdint>

#ifdef _WIN32
#include <direct.h>
#endif

// ============================================
// 构造函数
// 参数：windowSize - 滑动窗口大小，lookaheadSize - 前瞻缓冲区大小
// 说明：初始化压缩器参数，保存最近一次操作的统计信息
// ============================================
LZ77::LZ77(int windowSize, int lookaheadSize)
{
	m_windowSize = windowSize;
	m_lookaheadSize = lookaheadSize;
	m_lastOriginalSize = 0;
	m_lastCompressedSize = 0;
	m_lastTokenCount = 0;
}

// ============================================
// findLongestMatch: 在滑动窗口中寻找最长匹配
// 参数说明：
//   - data: 待压缩的完整字符串
//   - pos: 当前处理位置（索引）
// 返回：pair<偏移量, 匹配长度>
// 算法：
//   1. 确定窗口起始位置 = max(0, pos - windowSize)
//   2. 遍历窗口内每个可能的匹配起始位置
//   3. 计算从该位置开始与当前位置的最长匹配长度
//   4. 返回匹配长度最大的 (offset, length)
// 说明：偏移量 = pos - matchStartPos，确保在窗口范围内
// ============================================
std::pair<int, int> LZ77::findLongestMatch(const std::string& data, int pos)
{
	int maxLen = 0;   // 最长匹配长度
	int maxOffset = 0;  // 最长匹配的偏移量
	int windowStart = std::max(0, pos - m_windowSize);  // 窗口起始位置

	// 遍历窗口内所有可能的匹配起点
	for (int offset = 1; offset <= pos - windowStart; offset++)
	{
		int len = 0;  // 当前尝试的匹配长度

		// 尝试匹配最大长度（受前瞻缓冲区限制）
		while (len < m_lookaheadSize &&
			pos + len < (int)data.size() &&
			data[pos + len] == data[pos - offset + len])
		{
			len++;  // 匹配成功，长度+1
		}

		// 更新最长匹配记录
		if (len > maxLen)
		{
			maxLen = len;
			maxOffset = offset;
		}
	}

	return std::pair<int, int>(maxOffset, maxLen);
}

// ============================================
// compress: 压缩字符串
// 参数：input - 待压缩的原始字符串
// 返回：压缩后的字节向量
// 算法流程：
//   1. 从字符串起点开始，逐字符处理
//   2. 对每个位置，调用 findLongestMatch 找最长匹配
//   3. 生成 token：[offset(2B)][length(1B)][literal(1B)]
//   4. 如果有匹配，移动 (length + 1) 位；否则移动 1 位
//   5. 更新统计信息（原始大小、压缩后大小、token数量）
// 说明：短文本可能产生膨胀，因为每个 token 至少 4 字节
// ============================================
std::vector<char> LZ77::compress(const std::string& input)
{
	std::vector<char> result;  // 压缩结果
	int pos = 0;               // 当前处理位置
	int n = (int)input.size();  // 输入字符串长度
	m_lastTokenCount = 0;      // 重置 token 计数器

	while (pos < n)
	{
		// 在滑动窗口中查找最长匹配
		std::pair<int, int> match = findLongestMatch(input, pos);
		int offset = match.first;   // 偏移量
		int length = match.second;  // 匹配长度

		// 获取匹配后的下一个字符（Literal）
		char nextChar = (pos + length < n) ? input[pos + length] : '\0';

		// 写入 token（4字节）：
		// 第1-2字节：offset（小端序）
		// 第3字节：length
		// 第4字节：nextChar
		uint16_t offset16 = static_cast<uint16_t>(offset);
		result.push_back(static_cast<char>(offset16 & 0xFF));         // offset 低字节
		result.push_back(static_cast<char>((offset16 >> 8) & 0xFF));   // offset 高字节

		uint8_t len8 = static_cast<uint8_t>(length);
		result.push_back(static_cast<char>(len8));  // length

		result.push_back(nextChar);  // Literal

		// 移动指针：有匹配则跳 length+1，否则跳 1
		if (length == 0) pos++;
		else pos += length + 1;

		m_lastTokenCount++;  // 计数器+1
	}

	// 保存统计信息
	m_lastOriginalSize = input.size();
	m_lastCompressedSize = result.size();
	return result;
}

// ============================================
// decompress: 解压字节流
// 参数：compressed - 压缩后的字节向量
// 返回：解压后的原始字符串
// 算法流程：
//   1. 每 4 字节读取一个 token
//   2. 解析 offset 和 length
//   3. 根据 offset 从已有结果中复制 length 个字符
//   4. 追加 literal 字符（如果不是 '\0'）
//   5. 重复直到处理完毕
// 说明：offset 为 0 或超过结果长度时视为无效，跳过该引用
// ============================================
std::string LZ77::decompress(const std::vector<char>& compressed)
{
	std::string result;  // 解压结果
	int i = 0;           // 当前位置
	int n = (int)compressed.size();  // 数据长度

	while (i + 3 < n)
	{
		// 读取 token（4字节）
		// offset: 2字节（小端序）
		uint16_t offset = static_cast<unsigned char>(compressed[i]) |
			(static_cast<unsigned char>(compressed[i + 1]) << 8);
		uint8_t length = static_cast<unsigned char>(compressed[i + 2]);
		char nextChar = compressed[i + 3];
		i += 4;  // 移动到下一个 token

		// 处理向后引用
		if (length > 0)
		{
			// 安全检查：offset 不能超过已有数据长度
			// offset=0 本应表示 literal，但当前实现中 offset=0 表示无匹配
			if (offset == 0 || offset > (int)result.size()) {
				// 无效引用，跳过（不添加任何字符）
			}
			else {
				// 计算匹配起始位置 = 结果末尾 - offset
				int startPos = (int)result.size() - (int)offset;
				for (int j = 0; j < length; j++)
				{
					int idx = startPos + j;
					// 安全检查：确保索引在有效范围内
					if (idx >= 0 && idx < (int)result.size()) {
						result.push_back(result[idx]);
					}
				}
			}
		}

		// 追加 literal 字符（'\0' 表示无 literal）
		if (nextChar != '\0') result.push_back(nextChar);
	}

	return result;
}

// ============================================
// compressFile: 压缩文件
// 参数：
//   - inputPath: 源文件路径
//   - outputPath: 输出文件路径
// 返回：成功返回 true，失败返回 false
// 说明：
//   1. 读取源文件内容到字符串
//   2. 调用 compress() 压缩
//   3. 以二进制模式写入输出文件
// ============================================
bool LZ77::compressFile(const std::string& inputPath, const std::string& outputPath)
{
	// 读取源文件
	std::string content = readFileToString(inputPath);
	if (content.empty()) return false;

	// 压缩
	std::vector<char> compressed = compress(content);

	// 写入输出文件
	FILE* fp = nullptr;
	if (fopen_s(&fp, outputPath.c_str(), "wb") != 0 || fp == nullptr) return false;

	fwrite(compressed.data(), 1, compressed.size(), fp);
	fclose(fp);
	return true;
}

// ============================================
// decompressFile: 解压文件
// 参数：
//   - inputPath: 压缩文件路径
//   - outputPath: 输出文件路径
// 返回：成功返回 true，失败返回 false
// 说明：
//   1. 读取压缩文件到字节向量
//   2. 调用 decompress() 解压
//   3. 写入输出文件
// ============================================
bool LZ77::decompressFile(const std::string& inputPath, const std::string& outputPath)
{
	// 读取压缩文件
	std::vector<char> compressed;
	size_t size = readBytesFromFile(inputPath, compressed);
	if (size == 0) return false;

	// 解压
	std::string decompressed = decompress(compressed);
	return writeStringToFile(outputPath, decompressed);
}

// ============================================
// getCompressionRatio: 计算压缩率（百分比）
// 参数：originalSize - 原始大小，compressedSize - 压缩后大小
// 返回：compressedSize / originalSize * 100
// 说明：小于 100% 表示节省空间，大于 100% 表示膨胀
// ============================================
double LZ77::getCompressionRatio(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	return (static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;
}

// ============================================
// getSpaceSavingRate: 计算空间节省率（百分比）
// 参数：originalSize - 原始大小，compressedSize - 压缩后大小
// 返回：(1 - compressedSize / originalSize) * 100
// 说明：正值表示节省，负值表示占用更多空间
// ============================================
double LZ77::getSpaceSavingRate(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	return (1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;
}

// ============================================
// getExpansionRate: 计算膨胀率（百分比）
// 参数：originalSize - 原始大小，compressedSize - 压缩后大小
// 返回：如果压缩后更大，返回膨胀百分比；否则返回 0
// 说明：膨胀率 = max(0, (compressedSize - originalSize) / originalSize * 100)
// ============================================
double LZ77::getExpansionRate(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	double ratio = static_cast<double>(compressedSize) / static_cast<double>(originalSize);
	if (ratio <= 1.0) return 0.0;  // 没有膨胀
	return (ratio - 1.0) * 100.0;
}

// ============================================
// getLastStats: 获取最近一次压缩的统计信息
// 参数（输出）：
//   - originalSize: 原始数据大小
//   - compressedSize: 压缩后大小
//   - tokenCount: 生成的 token 数量
// 说明：每次 compress() 调用后更新这些值
// ============================================
void LZ77::getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const
{
	originalSize = m_lastOriginalSize;
	compressedSize = m_lastCompressedSize;
	tokenCount = m_lastTokenCount;
}

// ============================================
// setWindowSize: 设置滑动窗口大小
// 参数：size - 新的窗口大小（字节）
// 说明：窗口越大，可找到的匹配越长，但内存占用增加
// ============================================
void LZ77::setWindowSize(int size)
{
	m_windowSize = size;
}

// ============================================
// setLookaheadSize: 设置前瞻缓冲区大小
// 参数：size - 新的前瞻缓冲区大小（字节）
// 说明：前瞻缓冲区越大，最长匹配长度上限越高
// ============================================
void LZ77::setLookaheadSize(int size)
{
	m_lookaheadSize = size;
}