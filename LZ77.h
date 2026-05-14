#ifndef LZ77_H
#define LZ77_H

#include <vector>
#include <string>
#include <cstdint>

struct LZ77Token {
    int offset;
    int length;
    char nextChar;
};

class LZ77 {
public:
    LZ77(int windowSize = 4096, int lookaheadSize = 16);

    std::vector<char> compress(const std::string& input);
    bool compressFile(const std::string& inputPath, const std::string& outputPath);

    std::string decompress(const std::vector<char>& compressed);
    bool decompressFile(const std::string& inputPath, const std::string& outputPath);

    double getCompressionRatio(size_t originalSize, size_t compressedSize) const;
    void getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const;

    void setWindowSize(int size);
    void setLookaheadSize(int size);

private:
    std::pair<int, int> findLongestMatch(const std::string& data, int pos);

    int m_windowSize;
    int m_lookaheadSize;
    size_t m_lastOriginalSize;
    size_t m_lastCompressedSize;
    size_t m_lastTokenCount;
};

#endif