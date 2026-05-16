#ifndef LZ77_H
#define LZ77_H

// ============================================
// LZ77.h - LZ77 压缩算法头文件
// ============================================
// 功能：实现 LZ77 滑动窗口压缩算法，用于压缩假新闻文本
// 算法原理：利用历史数据中的重复字符串，用(偏移,长度)对替代
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <vector>
#include <string>
#include <cstdint>

// ============================================
// LZ77Token - LZ77 压缩令牌结构（当前未直接使用，供参考）
// 说明：LZ77 压缩的原子单位，每个 token 包含：
//   - offset: 向后引用偏移量（相对当前位置的窗口内距离）
//   - length: 匹配长度（连续重复的字符数）
//   - nextChar: 匹配后的下一个字符（Literal）
// 注：实际实现中直接序列化为 4 字节流，不使用此结构
// ============================================
struct LZ77Token {
	int offset;     // 向后引用偏移
	int length;     // 匹配长度
	char nextChar;  // 匹配后下一个字节
};

// ============================================
// LZ77 压缩器类
// 说明：自主实现 LZ77 压缩算法，零第三方依赖
// 核心参数：
//   - 窗口大小：历史数据搜索范围（默认 4096 字节）
//   - 前瞻缓冲区：最长匹配长度（默认 16 字节）
// ============================================
class LZ77 {
public:
	// ============================================
	// 构造函数
	// 参数说明：
	//   - windowSize: 滑动窗口大小（历史搜索范围）
	//   - lookaheadSize: 前瞻缓冲区大小（最大匹配长度）
	// ============================================
	LZ77(int windowSize = 4096, int lookaheadSize = 16);

	// ============================================
	// compress: 压缩内存字符串
	// 参数：input - 待压缩的原始字符串
	// 返回：压缩后的字节向量
	// 说明：使用 LZ77 算法进行压缩，每个 token 格式：
	//       [offset(2字节)][length(1字节)][literal(1字节)]
	// 算法流程：
	//   1. 从输入字符串起点开始扫描
	//   2. 在滑动窗口中查找最长匹配
	//   3. 输出 (偏移, 长度, 下一字符) 三元组
	//   4. 移动指针，重复直到处理完毕
	// ============================================
	std::vector<char> compress(const std::string& input);

	// ============================================
	// compressFile: 压缩文件
	// 参数：
	//   - inputPath: 源文件路径
	//   - outputPath: 输出文件路径（.ncz）
	// 返回：成功返回 true，失败返回 false
	// 说明：读取文件内容，调用 compress() 压缩，写入输出文件
	// ============================================
	bool compressFile(const std::string& inputPath, const std::string& outputPath);

	// ============================================
	// decompress: 解压内存字节流
	// 参数：compressed - 压缩后的字节向量
	// 返回：解压后的原始字符串
	// 说明：根据 token 序列重建原始数据
	// 算法流程：
	//   1. 读取每个 token 的 (偏移, 长度, 下一字符)
	//   2. 根据偏移从已有结果中复制匹配内容
	//   3. 追加下一字符到结果
	//   4. 重复直到处理完毕
	// ============================================
	std::string decompress(const std::vector<char>& compressed);

	// ============================================
	// decompressFile: 解压文件
	// 参数：
	//   - inputPath: 压缩文件路径（.ncz）
	//   - outputPath: 输出文件路径
	// 返回：成功返回 true，失败返回 false
	// 说明：读取压缩文件，调用 decompress() 解压，写入输出文件
	// ============================================
	bool decompressFile(const std::string& inputPath, const std::string& outputPath);

	// ============================================
	// getCompressionRatio: 计算压缩率
	// 参数：
	//   - originalSize: 原始数据大小（字节）
	//   - compressedSize: 压缩后大小（字节）
	// 返回：压缩率（百分比），即 compressed/original * 100
	// 说明：小于 100% 表示节省空间，大于 100% 表示数据膨胀
	// ============================================
	double getCompressionRatio(size_t originalSize, size_t compressedSize) const;

	// ============================================
	// getSpaceSavingRate: 计算空间节省率
	// 参数：
	//   - originalSize: 原始数据大小（字节）
	//   - compressedSize: 压缩后大小（字节）
	// 返回：节省率（百分比），即 (1 - compressed/original) * 100
	// 说明：正值表示节省空间，负值表示占用更多空间
	// ============================================
	double getSpaceSavingRate(size_t originalSize, size_t compressedSize) const;

	// ============================================
	// getExpansionRate: 计算数据膨胀率
	// 参数：
	//   - originalSize: 原始数据大小（字节）
	//   - compressedSize: 压缩后大小（字节）
	// 返回：膨胀率（百分比），即 max(0, compressed/original - 1) * 100
	// 说明：仅当压缩后大于原始时计算膨胀率，否则返回 0
	// ============================================
	double getExpansionRate(size_t originalSize, size_t compressedSize) const;

	// ============================================
	// getLastStats: 获取最近一次压缩的统计信息
	// 参数（输出）：
	//   - originalSize: 原始数据大小
	//   - compressedSize: 压缩后大小
	//   - tokenCount: 生成的 token 数量
	// 说明：每次 compress() 调用后更新这些值
	// ============================================
	void getLastStats(size_t& originalSize, size_t& compressedSize, size_t& tokenCount) const;

	// ============================================
	// 参数调节函数
	// ============================================
	void setWindowSize(int size);    // 设置窗口大小
	void setLookaheadSize(int size); // 设置前瞻缓冲区大小

private:
	// ============================================
	// findLongestMatch: 在滑动窗口中查找最长匹配
	// 参数说明：
	//   - data: 数据源（待压缩的完整字符串）
	//   - pos: 当前处理位置（字符索引）
	// 返回：pair<偏移量, 匹配长度>
	// 算法流程：
	//   1. 确定窗口起始位置（pos - windowSize）
	//   2. 遍历窗口内所有可能的起始位置
	//   3. 对每个位置，尝试匹配最大长度的连续字符
	//   4. 返回最长匹配的位置（偏移）和长度
	// ============================================
	std::pair<int, int> findLongestMatch(const std::string& data, int pos);

	// ============================================
	// 私有成员变量
	// ============================================
	int m_windowSize;         // 滑动窗口大小（历史搜索范围）
	int m_lookaheadSize;      // 前瞻缓冲区大小（最大匹配长度）
	size_t m_lastOriginalSize;     // 最近一次压缩的原始大小
	size_t m_lastCompressedSize;    // 最近一次压缩后的字节数
	size_t m_lastTokenCount;        // 最近一次生成的 token 数量
};

#endif // LZ77_H