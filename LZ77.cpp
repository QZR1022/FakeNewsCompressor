#define _CRT_SECURE_NO_WARNINGS
#include "LZ77.h"
#include "Utils.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdint>
using namespace std;

#ifdef _WIN32
#include <direct.h>
#endif

// 构造函数
LZ77::LZ77(int windowSize, int lookaheadSize)
{
	m_windowSize = windowSize;
	m_lookaheadSize = lookaheadSize;
	m_lastOriginalSize = 0;
	m_lastCompressedSize = 0;
	m_lastTokenCount = 0;
}

// 在滑动窗口中寻找最长匹配
pair<int, int> LZ77::findLongestMatch(const string& data, int pos)
{
	int maxLen = 0;
	int maxOffset = 0;
	int windowStart = max(0, pos - m_windowSize);

	for (int offset = 1; offset <= pos - windowStart; offset++)
	{
		int len = 0;
		while (len < m_lookaheadSize &&
			pos + len < (int)data.size() &&
			data[pos + len] == data[pos - offset + len])
		{
			len++;
		}

		if (len > maxLen)
		{
			maxLen = len;
			maxOffset = offset;
		}
	}

	return pair<int, int>(maxOffset, maxLen);
}

// 压缩字符串
vector<char> LZ77::compress(const string& input)
{
	vector<char> result;
	int pos = 0;
	int n = (int)input.size();
	m_lastTokenCount = 0;

	while (pos < n)
	{
		pair<int, int> match = findLongestMatch(input, pos);
		int offset = match.first;
		int length = match.second;

		char nextChar = (pos + length < n) ? input[pos + length] : '\0';

		// token格式：[offset(2B)][length(1B)][nextChar(1B)]
		uint16_t offset16 = static_cast<uint16_t>(offset);
		result.push_back(static_cast<char>(offset16 & 0xFF));
		result.push_back(static_cast<char>((offset16 >> 8) & 0xFF));

		uint8_t len8 = static_cast<uint8_t>(length);
		result.push_back(static_cast<char>(len8));

		result.push_back(nextChar);

		if (length == 0) pos++;
		else pos += length + 1;

		m_lastTokenCount++;
	}

	m_lastOriginalSize = input.size();
	m_lastCompressedSize = result.size();
	return result;
}

// 解压字节流
string LZ77::decompress(const vector<char>& compressed)
{
	string result;
	int i = 0;
	int n = (int)compressed.size();

	while (i + 3 < n)
	{
		uint16_t offset = static_cast<unsigned char>(compressed[i]) |
			(static_cast<unsigned char>(compressed[i + 1]) << 8);
		uint8_t length = static_cast<unsigned char>(compressed[i + 2]);
		char nextChar = compressed[i + 3];
		i += 4;

		if (length > 0)
		{
			if (offset == 0 || offset > result.size()) return "";

			int startPos = (int)result.size() - offset;
			for (int j = 0; j < length; j++)
			{
				int idx = startPos + j;
				if (idx < 0 || idx >= (int)result.size()) return "";
				result.push_back(result[idx]);
			}
		}

		if (nextChar != '\0') result.push_back(nextChar);
	}

	return result;
}

// 压缩文件
bool LZ77::compressFile(const string& inputPath, const string& outputPath)
{
	string content = readFileToString(inputPath);
	if (content.empty()) return false;

	vector<char> compressed = compress(content);

	FILE* fp = nullptr;
	if (fopen_s(&fp, outputPath.c_str(), "wb") != 0 || fp == nullptr) return false;

	fwrite(compressed.data(), 1, compressed.size(), fp);
	fclose(fp);
	return true;
}

// 解压文件
bool LZ77::decompressFile(const string& inputPath, const string& outputPath)
{
	vector<char> compressed;
	size_t size = readBytesFromFile(inputPath, compressed);
	if (size == 0) return false;

	string decompressed = decompress(compressed);
	return writeStringToFile(outputPath, decompressed);
}

// 压缩率（%）= compressed/original * 100
double LZ77::getCompressionRatio(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	return (static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;
}

// 空间节省率（%）= (1 - compressed/original) * 100
double LZ77::getSpaceSavingRate(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	return (1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;
}

// 膨胀率（%）= max(0, compressed/original - 1) * 100
double LZ77::getExpansionRate(size_t originalSize, size_t compressedSize) const
{
	if (originalSize == 0) return 0.0;
	double ratio = static_cast<double>(compressedSize) / static_cast<double>(originalSize);
	if (ratio <= 1.0) return 0.0;
	return (ratio - 1.0) * 100.0;
}

// 获取最后一次压缩统计
void LZ77::getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const
{
	originalSize = m_lastOriginalSize;
	compressedSize = m_lastCompressedSize;
	tokenCount = m_lastTokenCount;
}

// 设置窗口大小
void LZ77::setWindowSize(int size)
{
	m_windowSize = size;
}

// 设置前瞻缓冲区大小
void LZ77::setLookaheadSize(int size)
{
	m_lookaheadSize = size;
}