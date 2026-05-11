#include "PacketReader.h"
#include "Utils.h"

#include <fstream>
#include <string>

PacketReader::PacketReader() : m_currentIndex(0) {}

PacketReader::~PacketReader() {}

bool PacketReader::loadFile(const std::string& filePath) {
	m_newsList.clear();
	m_currentIndex = 0;

	std::ifstream in(filePath.c_str());
	if (!in.is_open()) {
		return false;
	}

	std::string line;
	while (std::getline(in, line)) {
		line = trim(line);
		if (line.empty()) {
			continue;
		}

		// 格式：timestamp|sourceIP|content
		size_t p1 = line.find('|');
		if (p1 == std::string::npos) {
			continue;
		}

		size_t p2 = line.find('|', p1 + 1);
		if (p2 == std::string::npos) {
			continue;
		}

		NewsPacket pkt;
		pkt.timestamp = trim(line.substr(0, p1));
		pkt.sourceIP = trim(line.substr(p1 + 1, p2 - p1 - 1));
		pkt.content = trim(line.substr(p2 + 1));

		if (pkt.timestamp.empty() || pkt.sourceIP.empty() || pkt.content.empty()) {
			continue;
		}

		m_newsList.push_back(pkt);
	}

	in.close();
	return !m_newsList.empty();
}

bool PacketReader::getNextNews(NewsPacket& packet) {
	if (!hasNext()) {
		return false;
	}

	packet = m_newsList[m_currentIndex];
	++m_currentIndex;
	return true;
}

void PacketReader::reset() {
	m_currentIndex = 0;
}

int PacketReader::getTotalCount() const {
	return static_cast<int>(m_newsList.size());
}

int PacketReader::getReadCount() const {
	return m_currentIndex;
}

bool PacketReader::hasNext() const {
	return m_currentIndex < static_cast<int>(m_newsList.size());
}

std::vector<NewsPacket> PacketReader::getAllNews() {
	return m_newsList;
}