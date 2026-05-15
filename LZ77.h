#ifndef LZ77_H
#define LZ77_H

#include <vector>
#include <string>
#include <cstdint>

// 可选：逻辑token结构（当前实现里直接序列化为4字节流）
// [offset(2B), length(1B), nextChar(1B)]
struct LZ77Token {
	int offset; // 向后引用偏移
	int length; // 匹配长度
	char nextChar; // 匹配后下一个字节
};

class LZ77 {
public:
	// 构造函数
	// windowSize: 滑动窗口大小
	// lookaheadSize: 前瞻缓冲区大小
	LZ77(int windowSize = 4096, int lookaheadSize = 16);

	// 压缩内存字符串 -> 压缩字节流
	std::vector<char> compress(const std::string& input);

	// 压缩文件
	bool compressFile(const std::string& inputPath, const std::string& outputPath);

	// 解压内存字节流 -> 原始字符串
	std::string decompress(const std::vector<char>& compressed);

	// 解压文件
	bool decompressFile(const std::string& inputPath, const std::string& outputPath);

	// 压缩率（Size Ratio, %）
	// = compressed/original * 100
	double getCompressionRatio(size_t originalSize, size_t compressedSize) const;

	// 空间节省率（Space Saving, %）
	// = (1 - compressed/original) * 100
	// >0 节省空间，<0 发生膨胀
	double getSpaceSavingRate(size_t originalSize, size_t compressedSize) const;

	// 膨胀率（Expansion, %）
	// = max(0, compressed/original - 1) * 100
	double getExpansionRate(size_t originalSize, size_t compressedSize) const;

	// 获取最近一次compress统计
	void getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const;

	// 参数调节
	void setWindowSize(int size);
	void setLookaheadSize(int size);

private:
	// 在窗口中找最长匹配，返回(offset, length)
	std::pair<int, int> findLongestMatch(const std::string& data, int pos);

	int m_windowSize;
	int m_lookaheadSize;
	size_t m_lastOriginalSize;
	size_t m_lastCompressedSize;
	size_t m_lastTokenCount;
};

#endif