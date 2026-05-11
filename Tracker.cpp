#include "Tracker.h"
#include "Utils.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <sstream>

Tracker::Tracker() {}

size_t Tracker::generateFingerprint(const std::string& content) {
	std::hash<std::string> hasher;
	return hasher(content);
}

void Tracker::recordPropagation(size_t fingerprint, const std::string& ip, const std::string& time) {
	PropagationRecord rec;
	rec.sourceIP = ip;
	rec.timestamp = time;

	m_propagationMap[fingerprint].push_back(rec);
}

int Tracker::getPropagationCount(size_t fingerprint) const {
	std::map<size_t, std::vector<PropagationRecord> >::const_iterator it = m_propagationMap.find(fingerprint);
	if (it == m_propagationMap.end()) return 0;
	return (int)it->second.size();
}

std::vector<PropagationRecord> Tracker::getPropagationHistory(size_t fingerprint) const {
	std::map<size_t, std::vector<PropagationRecord> >::const_iterator it = m_propagationMap.find(fingerprint);
	if (it == m_propagationMap.end()) {
		return std::vector<PropagationRecord>();
	}
	return it->second;
}

std::vector<size_t> Tracker::getAllFingerprints() const {
	std::vector<size_t> keys;
	for (std::map<size_t, std::vector<PropagationRecord> >::const_iterator it = m_propagationMap.begin();
		it != m_propagationMap.end(); ++it) {
		keys.push_back(it->first);
	}
	return keys;
}

std::vector<std::pair<size_t, int> > Tracker::getTopFakeNews(int topN) const {
	std::vector<std::pair<size_t, int> > list;
	if (topN <= 0) return list;

	for (std::map<size_t, std::vector<PropagationRecord> >::const_iterator it = m_propagationMap.begin();
		it != m_propagationMap.end(); ++it) {
		list.push_back(std::make_pair(it->first, (int)it->second.size()));
	}

	std::sort(list.begin(), list.end(),
		[](const std::pair<size_t, int>& a, const std::pair<size_t, int>& b) {
			if (a.second != b.second) return a.second > b.second; // 次数降序
			return a.first < b.first;
		});

	if ((int)list.size() > topN) {
		list.resize(topN);
	}

	return list;
}

bool Tracker::isTracked(size_t fingerprint) const {
	return m_propagationMap.find(fingerprint) != m_propagationMap.end();
}

bool Tracker::saveToFile(const std::string& filePath) const {
	// 文本格式：
	// fingerprint|timestamp|ip
	std::ofstream out(filePath.c_str(), std::ios::out | std::ios::trunc);
	if (!out.is_open()) return false;

	for (std::map<size_t, std::vector<PropagationRecord> >::const_iterator it = m_propagationMap.begin();
		it != m_propagationMap.end(); ++it) {
		size_t fp = it->first;
		const std::vector<PropagationRecord>& vec = it->second;

		for (size_t i = 0; i < vec.size(); ++i) {
			out << fp << "|" << vec[i].timestamp << "|" << vec[i].sourceIP << "\n";
		}
	}

	out.close();
	return true;
}

bool Tracker::loadFromFile(const std::string& filePath) {
	std::ifstream in(filePath.c_str());
	if (!in.is_open()) return false;

	m_propagationMap.clear();

	std::string line;
	while (std::getline(in, line)) {
		line = trim(line);
		if (line.empty()) continue;

		std::vector<std::string> parts = splitString(line, '|');
		if (parts.size() < 3) continue;

		// parts[0] = fingerprint
		// parts[1] = timestamp
		// parts[2] = ip
		std::istringstream iss(parts[0]);
		size_t fp = 0;
		iss >> fp;
		if (!iss) continue;

		PropagationRecord rec;
		rec.timestamp = parts[1];
		rec.sourceIP = parts[2];

		m_propagationMap[fp].push_back(rec);
	}

	in.close();
	return true;
}

void Tracker::clear() {
	m_propagationMap.clear();
}