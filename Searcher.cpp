#include "Searcher.h"
#include "Utils.h"

// ============================================
// Searcher.cpp - 关键词检索引擎实现文件
// ============================================
// 功能：实现基于倒排索引的关键词搜索
// 搜索策略：
//   1. 英文/数字：使用倒排索引（高效）
//   2. 中文：全文 fallback（子串匹配）
//   3. 短语搜索：全文子串匹配
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_map>

// ============================================
// 构造函数
// 说明：初始化空的索引和上下文
// ============================================
Searcher::Searcher() {}

// ============================================
// tokenize: 分词函数
// 参数：text - 待分词的文本
// 返回：词向量（英文/数字 token）
// 算法：
//   1. 遍历文本的每个字符
//   2. 收集连续的 isalnum 字符组成一个 token
//   3. 英文自动转小写
//   4. 遇到非字母数字字符时结束当前 token
// 说明：
//   - 英文和数字会被切分（如 "News2024" -> ["news", "2024"]）
//   - 中文不会被切分（按单个 UTF-8 字节处理）
//   - 标点符号作为分隔符
// ============================================
std::vector<std::string> Searcher::tokenize(const std::string& text) {
	std::vector<std::string> words;  // 分词结果
	std::string cur;                 // 当前正在收集的 token

	for (size_t i = 0; i < text.size(); ++i) {
		unsigned char ch = (unsigned char)text[i];
		if (std::isalnum(ch)) {
			// 是字母或数字，追加到当前 token
			cur.push_back((char)std::tolower(ch));
		}
		else {
			// 非字母数字，结束当前 token
			if (!cur.empty()) {
				words.push_back(cur);
				cur.clear();
			}
		}
	}

	// 处理最后一个 token
	if (!cur.empty()) {
		words.push_back(cur);
	}

	return words;
}

// ============================================
// isStopWord: 判断是否为停用词
// 参数：word - 待检查的词（小写形式）
// 返回：是停用词返回 true
// 说明：停用词列表包括常见英文虚词
//   - 冠词：the, a, an
//   - 系动词：is, are
//   - 介词：of, to, in, on, at
//   - 连词：and, or
// 停用词不加入倒排索引，节省空间
// ============================================
bool Searcher::isStopWord(const std::string& word) {
	static const std::set<std::string> stopWords = {
		"the", "a", "an", "is", "are", "of", "to", "and", "or", "in", "on", "at"
	};
	return stopWords.find(word) != stopWords.end();
}

// ============================================
// clearIndex: 清空索引和上下文
// 说明：重置所有内部数据结构
//   - 清空倒排索引
//   - 清空新闻正文、时间戳、可信度列表
// 用于准备重新构建索引
// ============================================
void Searcher::clearIndex() {
	m_invertedIndex.clear();
	m_newsContents.clear();
	m_newsTimestamps.clear();
	m_newsCredibilities.clear();
}

// ============================================
// buildIndex: 构建倒排索引
// 参数：newsContents - 新闻正文向量（按新闻编号索引）
// 说明：
//   1. 清空现有索引
//   2. 遍历所有新闻，对每条新闻进行分词
//   3. 将分词结果加入倒排索引
//   4. 跳过停用词
// 倒排索引格式：word -> [newsId1, newsId2, ...]
// 注意：只索引英文/数字 token，中文需要 fallback 搜索
// ============================================
void Searcher::buildIndex(const std::vector<std::string>& newsContents) {
	m_invertedIndex.clear();  // 清空现有索引
	m_newsContents = newsContents;  // 保存新闻正文

	for (int i = 0; i < (int)newsContents.size(); ++i) {
		addToIndex(i, newsContents[i]);
	}
}

// ============================================
// addToIndex: 向索引添加单条新闻
// 参数：
//   - newsId: 新闻编号（索引位置）
//   - content: 新闻正文
// 说明：
//   1. 确保 m_newsContents 容量足够（必要时扩展）
//   2. 对新闻内容进行分词
//   3. 将每个有效词加入倒排索引
//   4. 避免重复添加同一新闻编号
// ============================================
void Searcher::addToIndex(int newsId, const std::string& content) {
	if (newsId < 0) return;  // 无效编号

	// 确保容量足够
	if (newsId >= (int)m_newsContents.size()) {
		m_newsContents.resize(newsId + 1);
	}
	m_newsContents[newsId] = content;

	// 分词并加入倒排索引
	std::vector<std::string> words = tokenize(content);
	for (size_t i = 0; i < words.size(); ++i) {
		const std::string& w = words[i];
		if (w.empty()) continue;
		if (isStopWord(w)) continue;  // 跳过停用词

		// 获取该词的posting列表
		std::vector<int>& posting = m_invertedIndex[w];
		// 避免重复添加同一新闻编号
		if (posting.empty() || posting.back() != newsId) {
			posting.push_back(newsId);
		}
	}
}

// ============================================
// setNewsContext: 设置新闻上下文（时间、可信度）
// 参数：
//   - timestamps: 时间戳向量（与新闻编号对应）
//   - credibilities: 可信度向量（与新闻编号对应）
// 说明：在构建索引后调用，用于搜索结果展示
// ============================================
void Searcher::setNewsContext(const std::vector<std::string>& timestamps,
	const std::vector<double>& credibilities) {
	m_newsTimestamps = timestamps;
	m_newsCredibilities = credibilities;
}

// ============================================
// getDocumentCount: 获取已索引的新闻数量
// 返回：m_newsContents 中保存的新闻总数
// ============================================
int Searcher::getDocumentCount() const {
	return (int)m_newsContents.size();
}

// ============================================
// countOccurrences: 统计子串出现次数（静态辅助函数）
// 参数：
//   - lowText: 小写化的文本（搜索目标）
//   - key: 小写化的关键词（搜索词）
// 返回：key 在 lowText 中出现的次数
// 说明：用于全文 fallback 搜索
// ============================================
static int countOccurrences(const std::string& lowText, const std::string& key) {
	if (key.empty()) return 0;  // 空关键词
	int cnt = 0;
	size_t p = 0;
	while (true) {
		p = lowText.find(key, p);
		if (p == std::string::npos) break;
		++cnt;
		p += key.size();  // 移动到下次搜索位置
	}
	return cnt;
}

// ============================================
// search: 单关键词搜索
// 参数：keyword - 搜索关键词
// 返回：匹配的 SearchResult 向量
// 算法流程：
//   1. 关键词小写化并 trim
//   2. 优先在倒排索引中查找（高效路径）
//   3. 无结果时 fallback 到全文搜索（支持中文）
//   4. 按命中次数降序排序结果
// 说明：
//   - 英文关键词走倒排索引（高效）
//   - 中文关键词通常走 fallback（全文搜索）
// ============================================
std::vector<SearchResult> Searcher::search(const std::string& keyword) {
	std::vector<SearchResult> results;  // 结果列表

	// 处理关键词：小写化 + 去除首尾空格
	std::string key = toLower(trim(keyword));
	if (key.empty()) return results;  // 空关键词返回空结果

	// 优先查询倒排索引（高效路径）
	std::unordered_map<std::string, std::vector<int>>::iterator it =
		m_invertedIndex.find(key);
	if (it != m_invertedIndex.end()) {
		// 索引命中
		const std::vector<int>& docs = it->second;
		for (size_t i = 0; i < docs.size(); ++i) {
			int id = docs[i];
			if (id < 0 || id >= (int)m_newsContents.size()) continue;

			// 构造搜索结果
			SearchResult r;
			r.newsId = id;
			r.content = m_newsContents[id];
			r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
			r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
			r.usedFallback = false;  // 走索引路径

			// 计算命中次数（小写化后统计）
			std::string low = toLower(r.content);
			r.matchCount = countOccurrences(low, key);
			if (r.matchCount <= 0) r.matchCount = 1;  // 确保至少有 1

			results.push_back(r);
		}
	}
	else {
		// 索引无此词：全文搜索 fallback（中文关键词通常走这里）
		for (int id = 0; id < (int)m_newsContents.size(); ++id) {
			std::string low = toLower(m_newsContents[id]);
			int cnt = countOccurrences(low, key);
			if (cnt <= 0) continue;  // 无匹配跳过

			// 构造搜索结果
			SearchResult r;
			r.newsId = id;
			r.content = m_newsContents[id];
			r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
			r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
			r.matchCount = cnt;
			r.usedFallback = true;  // 走全文 fallback

			results.push_back(r);
		}
	}

	// 按命中次数降序排序
	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}

// ============================================
// searchMulti: 多关键词搜索（OR 模式）
// 参数：keywords - 关键词向量
// 返回：包含任一关键词的 SearchResult 向量
// 算法：
//   1. 对每个关键词，优先查倒排索引，fallback 到全文搜索
//   2. 累计每条新闻的命中关键词数量
//   3. 按命中数量降序排序
// 说明：
//   - matchCount 表示命中的关键词数量
//   - usedFallback 表示是否有任一关键词走 fallback
// ============================================
std::vector<SearchResult> Searcher::searchMulti(const std::vector<std::string>& keywords) {
	std::vector<SearchResult> results;
	if (keywords.empty()) return results;

	// docScore: 新闻ID -> 命中关键词数量
	std::map<int, int> docScore;
	// docFallback: 新闻ID -> 是否走过 fallback
	std::map<int, bool> docFallback;

	// 遍历每个关键词
	for (size_t i = 0; i < keywords.size(); ++i) {
		std::string key = toLower(trim(keywords[i]));
		if (key.empty()) continue;

		// 优先查倒排索引
		std::unordered_map<std::string, std::vector<int>>::iterator it =
			m_invertedIndex.find(key);
		if (it != m_invertedIndex.end()) {
			const std::vector<int>& docs = it->second;
			for (size_t j = 0; j < docs.size(); ++j) {
				int id = docs[j];
				if (id < 0 || id >= (int)m_newsContents.size()) continue;
				docScore[id] += 1;  // 命中数量 +1
			}
		}
		else {
			// fallback：全文搜索
			for (int id = 0; id < (int)m_newsContents.size(); ++id) {
				std::string low = toLower(m_newsContents[id]);
				if (low.find(key) != std::string::npos) {
					docScore[id] += 1;
					docFallback[id] = true;  // 标记走过 fallback
				}
			}
		}
	}

	// 构造结果列表
	for (std::map<int, int>::iterator mit = docScore.begin(); mit != docScore.end(); ++mit) {
		int id = mit->first;
		int score = mit->second;
		if (id < 0 || id >= (int)m_newsContents.size()) continue;

		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
		r.matchCount = score;
		r.usedFallback = (docFallback.find(id) != docFallback.end());

		results.push_back(r);
	}

	// 按命中数量降序排序
	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}

// ============================================
// searchExact: 精确短语搜索
// 参数：phrase - 搜索短语（忽略大小写）
// 返回：包含该短语的 SearchResult 向量
// 算法：
//   1. 短语小写化并 trim
//   2. 对每条新闻，检测是否包含该短语
//   3. 统计短语出现次数
//   4. 按出现次数降序排序
// 说明：直接做子串匹配，不使用倒排索引
// ============================================
std::vector<SearchResult> Searcher::searchExact(const std::string& phrase) {
	std::vector<SearchResult> results;

	// 处理短语：小写化 + 去除首尾空格
	std::string p = toLower(trim(phrase));
	if (p.empty()) return results;

	// 遍历所有新闻
	for (int id = 0; id < (int)m_newsContents.size(); ++id) {
		std::string low = toLower(m_newsContents[id]);
		size_t pos = low.find(p);
		if (pos == std::string::npos) continue;  // 无匹配跳过

		// 构造搜索结果
		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
		r.usedFallback = false;  // exact 不算 fallback

		// 统计出现次数
		r.matchCount = 0;
		while (pos != std::string::npos) {
			++r.matchCount;
			pos = low.find(p, pos + p.size());  // 移动到下次搜索位置
		}
		if (r.matchCount <= 0) r.matchCount = 1;  // 确保至少有 1

		results.push_back(r);
	}

	// 按出现次数降序排序
	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}