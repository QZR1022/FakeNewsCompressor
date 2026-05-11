#ifndef LZ77_H
#define LZ77_H

#include <vector>
#include <string>
#include <utility>

// LZ77 压缩的令牌（Token）
struct LZ77Token {
    int offset;      // 匹配距离（从当前位置往前多少字节）
    int length;      // 匹配长度
    char nextChar;   // 下一个字符
};

// LZ77 压缩器类
class LZ77 {
public:
    // 构造函数：可自定义窗口和前瞻缓冲区大小
    // 参数：windowSize - 滑动窗口大小，lookaheadSize - 前瞻缓冲区大小
    LZ77(int windowSize = 4096, int lookaheadSize = 16);

    // 压缩字符串
    // 参数：input - 原始字符串
    // 返回：压缩后的字节流
    std::vector<char> compress(const std::string& input);

    // 压缩文件
    // 参数：inputPath - 原始文件路径，outputPath - 压缩后文件路径
    // 返回：成功返回true
    bool compressFile(const std::string& inputPath, const std::string& outputPath);

    // 解压字节流
    // 参数：compressed - 压缩后的字节流
    // 返回：解压后的字符串
    std::string decompress(const std::vector<char>& compressed);

    // 解压文件
    // 参数：inputPath - 压缩文件路径，outputPath - 解压后文件路径
    // 返回：成功返回true
    bool decompressFile(const std::string& inputPath, const std::string& outputPath);

    // 计算压缩率
    // 参数：originalSize - 原始大小，compressedSize - 压缩后大小
    // 返回：压缩率（0-100），例如 60 表示压缩到原大小的60%
    double getCompressionRatio(size_t originalSize, size_t compressedSize) const;

    // 获取最后一次压缩的详细统计
    // 参数：originalSize - 输出原始大小，compressedSize - 输出压缩后大小，tokenCount - 输出令牌数量
    void getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const;

    // 设置滑动窗口大小
    void setWindowSize(int size);

    // 设置前瞻缓冲区大小
    void setLookaheadSize(int size);

private:
    int m_windowSize;       // 滑动窗口大小
    int m_lookaheadSize;    // 前瞻缓冲区大小

    // 统计信息
    size_t m_lastOriginalSize;
    size_t m_lastCompressedSize;
    size_t m_lastTokenCount;

    // 在窗口中找到最长匹配
    // 参数：data - 数据，pos - 当前位置
    // 返回：最佳匹配的(offset, length)
    std::pair<int, int> findLongestMatch(const std::string& data, int pos);
};

#endif // LZ77_H

