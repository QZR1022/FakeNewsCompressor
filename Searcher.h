#ifndef SEARCHER_H
#define SEARCHER_H

// ============================================
// Searcher.h - 关键词检索引擎头文件
// ============================================
// 功能：提供基于倒排索引的关键词搜索，支持中英文混合检索
// 特点：
//   - 英文/数字使用分词 + 倒排索引（高效）
//   - 中文使用全文 fallback（支持但效率较低）
//   - 支持精确短语搜索
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>
#include <vector>
#include <unordered_map>

// ============================================
// SearchResult - 单条检索结果结构
// ============================================
struct SearchResult {
	int newsId;              // 新闻编号（索引）
	std::string content;      // 新闻正文
	std::string timestamp;    // 时间戳
	double credibility;       // 可信度（0-100）
	int matchCount;          // 关键词命中次数
	bool usedFallback;        // 是否使用了全文 fallback
};

// ============================================
// Searcher - 检索引擎类
// 功能：
//   - 构建倒排索引
//   - 执行关键词搜索
//   - 存储新闻上下文（时间、可信度）
// 说明：
//   - 索引在监控模式结束后构建
//   - 检索时优先使用倒排索引，无结果时 fallback 到全文搜索
// ============================================
class Searcher {
public:
	// ============================================
	// 构造函数
	// 说明：初始化空的索引和上下文
	// ============================================
	Searcher();

	// ============================================
	// clearIndex: 清空索引和上下文
	// 说明：重置所有内部数据结构，准备重新构建索引
	// ============================================
	void clearIndex();

	// ============================================
	// buildIndex: 构建倒排索引
	// 参数：newsContents - 新闻正文向量
	// 说明：
	//   1. 清空现有索引
	//   2. 遍历所有新闻，对每条新闻进行分词
	//   3. 将分词结果加入倒排索引（word -> [newsId1, newsId2, ...]）
	//   4. 跳过停用词（如 the, a, is 等）
	// ============================================
	void buildIndex(const std::vector<std::string>& newsContents);

	// ============================================
	// addToIndex: 向索引添加单条新闻
	// 参数：
	//   - newsId: 新闻编号
	//   - content: 新闻正文
	// 说明：用于增量添加新闻到索引（当前版本未使用）
	// ============================================
	void addToIndex(int newsId, const std::string& content);

	// ============================================
	// setNewsContext: 设置新闻上下文（时间、可信度）
	// 参数：
	//   - timestamps: 时间戳向量（与新闻编号对应）
	//   - credibilities: 可信度向量（与新闻编号对应）
	// 说明：在构建索引后调用，用于搜索结果展示
	// ============================================
	void setNewsContext(const std::vector<std::string>& timestamps,
		const std::vector<double>& credibilities);

	// ============================================
	// getDocumentCount: 获取已索引的新闻数量
	// 返回：索引中的新闻总数
	// ============================================
	int getDocumentCount() const;

	// ============================================
	// search: 单关键词搜索
	// 参数：keyword - 搜索关键词
	// 返回：匹配的 SearchResult 向量
	// 算法流程：
	//   1. 优先在倒排索引中查找（高效）
	//   2. 无结果时 fallback 到全文搜索（支持中文）
	//   3. 按命中次数降序排序
	// ============================================
	std::vector<SearchResult> search(const std::string& keyword);

	// ============================================
	// searchMulti: 多关键词搜索（OR）
	// 参数：keywords - 关键词向量
	// 返回：包含任一关键词的 SearchResult 向量
	// 说明：每条新闻的 matchCount 表示命中关键词数量
	// ============================================
	std::vector<SearchResult> searchMulti(const std::vector<std::string>& keywords);

	// ============================================
	// searchExact: 精确短语搜索
	// 参数：phrase - 搜索短语（忽略大小写）
	// 返回：包含该短语的 SearchResult 向量
	// 说明：直接在全文中做子串匹配，不使用倒排索引
	// ============================================
	std::vector<SearchResult> searchExact(const std::string& phrase);

private:
	// ============================================
	// tokenize: 分词函数
	// 参数：text - 待分词的文本
	// 返回：词向量（英文/数字 token）
	// 说明：
	//   - 连续 isalnum 字符组成一个 token
	//   - 英文自动转小写
	//   - 中文不会被切分（需要额外处理）
	// ============================================
	std::vector<std::string> tokenize(const std::string& text);

	// ============================================
	// isStopWord: 判断是否为停用词
	// 参数：word - 待检查的词
	// 返回：是停用词返回 true
	// 说明：停用词列表：the, a, an, is, are, of, to, and, or, in, on, at
	// ============================================
	bool isStopWord(const std::string& word);

	// ============================================
	// 私有成员变量
	// ============================================
	std::unordered_map<std::string, std::vector<int>> m_invertedIndex;  // 倒排索引
	std::vector<std::string> m_newsContents;   // 新闻正文列表
	std::vector<std::string> m_newsTimestamps;  // 时间戳列表
	std::vector<double> m_newsCredibilities;   // 可信度列表
};

#endif // SEARCHER_H