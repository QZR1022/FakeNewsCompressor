#include "LZ77.h"
#include "Utils.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

static void WriteU16(std::vector<char>& out, unsigned short v) {
	out.push_back((char)(v & 0xFF));
	out.push_back((char)((v >> 8) & 0xFF));
}

static bool ReadU16(const std::vector<char>& in, size_t& p, unsigned short& v) {
	if (p + 1 >= in.size()) return false;
	unsigned char b0 = (unsigned char)in[p];
	unsigned char b1 = (unsigned char)in[p + 1];
	v = (unsigned short)(b0 | (b1 << 8));
	p += 2;
	return true;
}

LZ77::LZ77(int windowSize, int lookaheadSize)
	: m_windowSize(windowSize),
	m_lookaheadSize(lookaheadSize),
	m_lastOriginalSize(0),
	m_lastCompressedSize(0),
	m_lastTokenCount(0) {
	if (m_windowSize <= 0) m_windowSize = 4096;
	if (m_lookaheadSize <= 0) m_lookaheadSize = 16;
}

std::pair<int, int> LZ77::findLongestMatch(const std::string& data, int pos) {
	int bestOffset = 0;
	int bestLen = 0;

	int start = std::max(0, pos - m_windowSize);
	int maxLen = std::min(m_lookaheadSize, (int)data.size() - pos);

	for (int i = start; i < pos; ++i) {
		int len = 0;
		while (len < maxLen && (i + len) < pos && data[i + len] == data[pos + len]) {
			++len;
		}
		if (len > bestLen) {
			bestLen = len;
			bestOffset = pos - i;
		}
	}

	return std::make_pair(bestOffset, bestLen);
}

std::vector<char> LZ77::compress(const std::string& input) {
	std::vector<char> out;
	m_lastOriginalSize = input.size();
	m_lastTokenCount = 0;

	int pos = 0;
	while (pos < (int)input.size()) {
		std::pair<int, int> m = findLongestMatch(input, pos);
		int offset = m.first;
		int len = m.second;

		if (pos + len >= (int)input.size()) {
			if (len > 0) --len;
			else offset = 0;
		}

		char nextChar = input[pos + len];

		WriteU16(out, (unsigned short)offset);
		WriteU16(out, (unsigned short)len);
		out.push_back(nextChar);

		++m_lastTokenCount;
		pos += (len + 1);
	}

	m_lastCompressedSize = out.size();
	return out;
}

std::string LZ77::decompress(const std::vector<char>& compressed) {
	std::string out;
	size_t p = 0;

	while (p < compressed.size()) {
		unsigned short offset = 0;
		unsigned short len = 0;

		if (!ReadU16(compressed, p, offset)) break;
		if (!ReadU16(compressed, p, len)) break;
		if (p >= compressed.size()) break;

		char nextChar = compressed[p++];
		if (offset > 0 && len > 0) {
			if ((size_t)offset > out.size()) continue;
			size_t start = out.size() - (size_t)offset;
			for (unsigned short i = 0; i < len; ++i) {
				out.push_back(out[start + i]);
			}
		}
		out.push_back(nextChar);
	}

	return out;
}

bool LZ77::compressFile(const std::string& inputPath, const std::string& outputPath) {
	std::string s = readFileToString(inputPath);
	if (s.empty() && !fileExists(inputPath)) return false;

	std::vector<char> c = compress(s);
	if (c.empty() && !s.empty()) return false;

	return writeBytesToFile(outputPath, c.data(), c.size());
}

bool LZ77::decompressFile(const std::string& inputPath, const std::string& outputPath) {
	std::vector<char> c;
	size_t n = readBytesFromFile(inputPath, c);
	if (n == 0 && !fileExists(inputPath)) return false;

	std::string s = decompress(c);
	return writeStringToFile(outputPath, s);
}

double LZ77::getCompressionRatio(size_t originalSize, size_t compressedSize) const {
	if (originalSize == 0) return 0.0;
	return (double)compressedSize * 100.0 / (double)originalSize;
}

void LZ77::getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const {
	originalSize = m_lastOriginalSize;
	compressedSize = m_lastCompressedSize;
	tokenCount = m_lastTokenCount;
}

void LZ77::setWindowSize(int size) {
	if (size > 0) m_windowSize = size;
}

void LZ77::setLookaheadSize(int size) {
	if (size > 0) m_lookaheadSize = size;
}