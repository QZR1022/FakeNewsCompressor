#include "Searcher.h"
#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

Searcher::Searcher() {}

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

std::vector<SearchResult> Searcher::search(const std::string& keyword) {
	std::vector<SearchResult> results;

	std::string key = toLower(trim(keyword));
	if (key.empty()) return results;

	std::unordered_map<std::string, std::vector<int> >::iterator it = m_invertedIndex.find(key);
	if (it == m_invertedIndex.end()) return results;

	const std::vector<int>& docs = it->second;
	for (size_t i = 0; i < docs.size(); ++i) {
		int id = docs[i];
		if (id < 0 || id >= (int)m_newsContents.size()) continue;

		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;

		// 统计匹配次数
		r.matchCount = 0;
		std::string low = toLower(r.content);
		size_t p = 0;
		while (true) {
			p = low.find(key, p);
			if (p == std::string::npos) break;
			++r.matchCount;
			p += key.size();
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

std::vector<SearchResult> Searcher::searchMulti(const std::vector<std::string>& keywords) {
	std::vector<SearchResult> results;
	if (keywords.empty()) return results;

	std::map<int, int> docScore;

	for (size_t i = 0; i < keywords.size(); ++i) {
		std::string key = toLower(trim(keywords[i]));
		if (key.empty()) continue;

		std::unordered_map<std::string, std::vector<int> >::iterator it = m_invertedIndex.find(key);
		if (it == m_invertedIndex.end()) continue;

		const std::vector<int>& docs = it->second;
		for (size_t j = 0; j < docs.size(); ++j) {
			int id = docs[j];
			if (id < 0 || id >= (int)m_newsContents.size()) continue;
			docScore[id] += 1;
		}
	}

	std::map<int, int>::iterator mit;
	for (mit = docScore.begin(); mit != docScore.end(); ++mit) {
		int id = mit->first;
		int score = mit->second;

		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
		r.matchCount = score;

		results.push_back(r);
	}

	std::sort(results.begin(), results.end(),
		[](const SearchResult& a, const SearchResult& b) {
			if (a.matchCount != b.matchCount) return a.matchCount > b.matchCount;
			return a.newsId < b.newsId;
		});

	return results;
}

std::vector<SearchResult> Searcher::searchExact(const std::string& phrase) {
	std::vector<SearchResult> results;

	std::string p = trim(phrase);
	if (p.empty()) return results;

	for (int id = 0; id < (int)m_newsContents.size(); ++id) {
		if (m_newsContents[id].find(p) == std::string::npos) continue;

		SearchResult r;
		r.newsId = id;
		r.content = m_newsContents[id];
		r.timestamp = (id < (int)m_newsTimestamps.size()) ? m_newsTimestamps[id] : "";
		r.credibility = (id < (int)m_newsCredibilities.size()) ? m_newsCredibilities[id] : 0.0;
		r.matchCount = 1;

		results.push_back(r);
	}

	return results;
}