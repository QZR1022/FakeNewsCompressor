#include "Searcher.h"
#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

Searcher::Searcher() {}

/*
* 分词函数（当前主要面向英文/数字）
* - 连续 isalnum 字符组成一个 token
* - 中文不会被切成 token（因此中文关键词一般不在倒排索引里）
*/
std::vector<std::string> Searcher::tokenize(const std::string& text) {
	std::vector<std::string> words;
	std::string cur;

	for (size_t i = 0; i < text.size(); ++i) {
		unsigned char ch = (unsigned char)text[i];
		if (std::isalnum(ch)) {
			cur.push_back((char)std::tolower(ch));
		}
		else {
			if (!cur.empty()) {
				words.push_back(cur);
				cur.clear();
			}
		}
	}

	if (!cur.empty()) {
		words.push_back(cur);
	}

	return words;
}

bool Searcher::isStopWord(const std::string& word) {
	static const std::set<std::string> stopWords = {
	"the", "a", "an", "is", "are", "of", "to", "and", "or", "in", "on", "at"
	};
	return stopWords.find(word) != stopWords.end();
}

void Searcher::clearIndex() {
	m_invertedIndex.clear();
	m_newsContents.clear();
	m_newsTimestamps.clear();
	m_newsCredibilities.clear();
}

void Searcher::buildIndex(const std::vector<std::string>& newsContents) {
	m_invertedIndex.clear();
	m_newsContents = newsContents;

	for (int i = 0; i < (int)newsContents.size(); ++i) {
		addToIndex(i, newsContents[i]);
	}
}

void Searcher::addToIndex(int newsId, const std::string& content) {
	if (newsId < 0) return;

	if (newsId >= (int)m_newsContents.size()) {
		m_newsContents.resize(newsId + 1);
	}
	m_newsContents[newsId] = content;

	// 倒排索引：只索引 token（英文/数字）
	std::vector<std::string> words = tokenize(content);
	for (size_t i = 0; i < words.size(); ++i) {
		const std::string& w = words[i];
		if (w.empty()) continue;
		if (isStopWord(w)) continue;

		std::vector<int>& posting = m_invertedIndex[w];
		if (posting.empty() || posting.back() != newsId) {
			posting.push_back(newsId);
		}
	}
}

void Searcher::setNewsContext(const std::vector<std::string>& timestamps,
	const std::vector<double>& credibilities) {
	m_newsTimestamps = timestamps;
	m_newsCredibilities = credibilities;
}

int Searcher::getDocumentCount() const {
	return (int)m_newsContents.size();
}

// 统计 key 在 lowText 中出现次数
static int countOccurrences(const std::string& lowText, const std::string& key) {
	if (key.empty()) return 0;
	int cnt = 0;
	size_t p = 0;
	while (true) {
		p = lowText.find(key, p);
		if (p == std::string::npos) break;
		++cnt;
		p += key.size();
	}
	return cnt;
}

/*
* 单关键词搜索：
* 1) 优先倒排索引（快）
* 2) 倒排无命中时，fallback 全文子串匹配（支持中文）
*/
std::vector<SearchResult> Searcher::search(const std::string& keyword) {
	std::vector<SearchResult> results;

	std::string key = toLower(trim(keyword));
	if (key.empty()) return results;

	// 先查倒排索引
	std::unordered_map<std::string, std::vector<int> >::iterator it = m_invertedIndex.find(key);
	if (it != m_invertedIndex.end()) {
		const std::vector<int>& docs = it->second;
		for (size_t i = 0; i < docs.size(); ++i) {
			int id = docs[i];
			if (id < 0 || id >= (int)m_newsContents.size()) continue;

			SearchResult r;
			r.newsId = id;
			r.content = m_newsContents[id];
			r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
			r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
			r.usedFallback = false; // 走索引

			std::string low = toLower(r.content);
			r.matchCount = countOccurrences(low, key);
			if (r.matchCount <= 0) r.matchCount = 1;

			results.push_back(r);
		}
	}
	else {
		// 索引无此词：全文匹配兜底（中文关键词通常走这里）
		for (int id = 0; id < (int)m_newsContents.size(); ++id) {
			std::string low = toLower(m_newsContents[id]);
			int cnt = countOccurrences(low, key);
			if (cnt <= 0) continue;

			SearchResult r;
			r.newsId = id;
			r.content = m_newsContents[id];
			r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
			r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
			r.matchCount = cnt;
			r.usedFallback = true; // 走全文fallback

			results.push_back(r);
		}
	}

	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}

/*
* 多关键词搜索（OR）：
* - 每个关键词优先倒排索引
* - 索引无该词时，全文匹配兜底
* - matchCount 表示命中“关键词数量”
*/
std::vector<SearchResult> Searcher::searchMulti(const std::vector<std::string>& keywords) {
	std::vector<SearchResult> results;
	if (keywords.empty()) return results;

	std::map<int, int> docScore; // newsId -> 命中关键词数
	std::map<int, bool> docFallback; // newsId -> 是否有任一关键词走过fallback

	for (size_t i = 0; i < keywords.size(); ++i) {
		std::string key = toLower(trim(keywords[i]));
		if (key.empty()) continue;

		std::unordered_map<std::string, std::vector<int> >::iterator it = m_invertedIndex.find(key);
		if (it != m_invertedIndex.end()) {
			const std::vector<int>& docs = it->second;
			for (size_t j = 0; j < docs.size(); ++j) {
				int id = docs[j];
				if (id < 0 || id >= (int)m_newsContents.size()) continue;
				docScore[id] += 1;
			}
		}
		else {
			for (int id = 0; id < (int)m_newsContents.size(); ++id) {
				std::string low = toLower(m_newsContents[id]);
				if (low.find(key) != std::string::npos) {
					docScore[id] += 1;
					docFallback[id] = true;
				}
			}
		}
	}

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

	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}

/*
* 精确短语搜索（忽略大小写）
* - 直接对 lower 后文本做子串匹配
*/
std::vector<SearchResult> Searcher::searchExact(const std::string& phrase) {
	std::vector<SearchResult> results;

	std::string p = toLower(trim(phrase));
	if (p.empty()) return results;

	for (int id = 0; id < (int)m_newsContents.size(); ++id) {
		std::string low = toLower(m_newsContents[id]);
		size_t pos = low.find(p);
		if (pos == std::string::npos) continue;

		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
		r.usedFallback = false; // exact 不算 fallback

		r.matchCount = 0;
		while (pos != std::string::npos) {
			++r.matchCount;
			pos = low.find(p, pos + p.size());
		}
		if (r.matchCount <= 0) r.matchCount = 1;

		results.push_back(r);
	}

	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}