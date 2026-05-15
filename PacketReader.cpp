#include "PacketReader.h"

#include <fstream>
#include <string>
#include <vector>

// 构造函数
PacketReader::PacketReader() : m_currentIndex(0) {}

// 析构函数
PacketReader::~PacketReader() {
    // 当前实现不持久占用文件句柄，无需额外释放
}

// 加载新闻文件
bool PacketReader::loadFile(const std::string& filePath) {
    m_newsList.clear();
    m_currentIndex = 0;

    std::ifstream fin(filePath.c_str());
    if (!fin.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        // 格式：timestamp|ip|content
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;

        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;

        NewsPacket pkt;
        pkt.timestamp = line.substr(0, p1);
        pkt.sourceIP = line.substr(p1 + 1, p2 - (p1 + 1));
        pkt.content = line.substr(p2 + 1);

        if (pkt.timestamp.empty() || pkt.sourceIP.empty() || pkt.content.empty()) {
            continue;
        }

        m_newsList.push_back(pkt);
    }

    fin.close();
    return true;
}

// 获取下一条新闻
bool PacketReader::getNextNews(NewsPacket& packet) {
    if (!hasNext()) {
        return false;
    }

    packet = m_newsList[m_currentIndex];
    m_currentIndex++;
    return true;
}

// 重置读取位置
void PacketReader::reset() {
    m_currentIndex = 0;
}

// 获取新闻总数
int PacketReader::getTotalCount() const {
    return static_cast<int>(m_newsList.size());
}

// 获取已读取数量
int PacketReader::getReadCount() const {
    return m_currentIndex;
}

// 是否还有更多新闻
bool PacketReader::hasNext() const {
    return m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_newsList.size());
}

// 获取所有新闻
std::vector<NewsPacket> PacketReader::getAllNews() {
    return m_newsList;
}