#ifndef SEARCHER_H
#define SEARCHER_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

// 搜索结果项
struct SearchResult {
	int newsId; // 新闻ID
	std::string content; // 新闻内容
	std::string timestamp; // 时间戳
	double credibility; // 可信度
	int matchCount; // 关键词匹配次数
	bool usedFallback; // 是否走了全文fallback匹配（true=全文匹配，false=倒排索引）
};

// 检索器类
class Searcher {
public:
	// 构造函数
	Searcher();

	// 构建倒排索引
	// 参数：newsContents - 所有新闻正文列表
	void buildIndex(const std::vector<std::string>& newsContents);

	// 添加单条新闻到索引
	// 参数：newsId - 新闻ID，content - 新闻正文
	void addToIndex(int newsId, const std::string& content);

	// 搜索关键词
	// 参数：keyword - 搜索关键词
	// 返回：匹配的搜索结果列表（按匹配次数降序）
	std::vector<SearchResult> search(const std::string& keyword);

	// 多关键词搜索（OR逻辑，包含任一关键词即可）
	// 参数：keywords - 关键词列表
	// 返回：匹配的搜索结果列表
	std::vector<SearchResult> searchMulti(const std::vector<std::string>& keywords);

	// 精确搜索（整个短语，忽略大小写）
	// 参数：phrase - 搜索短语
	// 返回：匹配的搜索结果列表
	std::vector<SearchResult> searchExact(const std::string& phrase);

	// 清空索引
	void clearIndex();

	// 获取索引中的文档数量
	int getDocumentCount() const;

	// 设置新闻上下文（用于返回搜索结果时附带详细信息）
	// 参数：timestamps - 时间戳列表，credibilities - 可信度列表
	void setNewsContext(const std::vector<std::string>& timestamps,
		const std::vector<double>& credibilities);

private:
	// 倒排索引：关键词 -> 新闻ID列表（主要适用于英文/数字token）
	std::unordered_map<std::string, std::vector<int>> m_invertedIndex;

	// 存储新闻内容（用于返回结果）
	std::vector<std::string> m_newsContents;
	std::vector<std::string> m_newsTimestamps;
	std::vector<double> m_newsCredibilities;

	// 辅助函数：提取文本中的所有单词（当前主要英文/数字）
	std::vector<std::string> tokenize(const std::string& text);

	// 辅助函数：停用词过滤（英文）
	bool isStopWord(const std::string& word);
};

#endif // SEARCHER_H