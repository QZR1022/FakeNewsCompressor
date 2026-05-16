#include "PacketReader.h"

#include <fstream>
#include <string>
#include <vector>

// ============================================
// PacketReader.cpp - 新闻数据读取器实现文件
// ============================================
// 功能：从 news.txt 文件读取新闻数据包
// 文件格式：timestamp|IP|content（竖线分隔）
// 作者：QZR1022
// 版本：1.0
// ============================================

// ============================================
// 构造函数
// 说明：初始化读取位置为 0
// ============================================
PacketReader::PacketReader() : m_currentIndex(0) {}

// ============================================
// 析构函数
// 说明：当前实现不持久占用文件句柄，无需额外释放
// 文件在 loadFile() 中打开后立即关闭，读取时从内存列表中获取
// ============================================
PacketReader::~PacketReader() {
	// 当前版本无需要释放的资源
}

// ============================================
// loadFile: 加载新闻文件
// 参数：filePath - 新闻文件路径
// 返回：成功返回 true，失败返回 false
// 说明：
//   1. 清空现有数据（支持重新加载）
//   2. 重置读取位置
//   3. 打开文件，逐行读取
//   4. 解析格式：timestamp|IP|content
//   5. 跳过空行和格式错误的行
// 格式示例：
//   2026-05-14 10:00:01|192.168.1.1|震惊！林丹决赛前药检阳性！
// ============================================
bool PacketReader::loadFile(const std::string& filePath) {
	m_newsList.clear();  // 清空现有数据
	m_currentIndex = 0;  // 重置读取位置

	// 打开文件（文本模式）
	std::ifstream fin(filePath.c_str());
	if (!fin.is_open()) {
		return false;  // 文件打开失败
	}

	std::string line;
	while (std::getline(fin, line)) {
		if (line.empty()) continue;  // 跳过空行

		// 解析格式：timestamp|IP|content
		// 需要找到两个竖线的位置
		size_t p1 = line.find('|');  // 第一个竖线位置
		if (p1 == std::string::npos) continue;  // 格式错误，跳过

		size_t p2 = line.find('|', p1 + 1);  // 第二个竖线位置
		if (p2 == std::string::npos) continue;  // 格式错误，跳过

		// 提取各字段
		NewsPacket pkt;
		pkt.timestamp = line.substr(0, p1);                              // 时间戳
		pkt.sourceIP = line.substr(p1 + 1, p2 - (p1 + 1));               // IP 地址
		pkt.content = line.substr(p2 + 1);                              // 新闻正文

		// 验证字段非空
		if (pkt.timestamp.empty() || pkt.sourceIP.empty() || pkt.content.empty()) {
			continue;  // 字段为空，跳过
		}

		m_newsList.push_back(pkt);  // 添加到列表
	}

	fin.close();  // 关闭文件
	return true;
}

// ============================================
// getNextNews: 获取下一条新闻
// 参数：packet - 输出参数，用于接收新闻数据
// 返回：有数据返回 true，无数据返回 false
// 说明：
//   - 按顺序返回新闻列表中的下一条
//   - 每次调用后内部指针 m_currentIndex 前移
//   - 用于 while 循环遍历所有新闻
// 使用示例：
//   NewsPacket pkt;
//   while (reader.getNextNews(pkt)) {
//       // 处理 pkt
//   }
// ============================================
bool PacketReader::getNextNews(NewsPacket& packet) {
	if (!hasNext()) {  // 没有更多新闻
		return false;
	}

	packet = m_newsList[m_currentIndex];  // 获取当前新闻
	m_currentIndex++;  // 指针前移
	return true;
}

// ============================================
// reset: 重置读取位置
// 说明：将内部指针 m_currentIndex 重置到 0
//       可在加载文件后重新遍历
// ============================================
void PacketReader::reset() {
	m_currentIndex = 0;
}

// ============================================
// getTotalCount: 获取新闻总数
// 返回：m_newsList 中保存的新闻数量
// ============================================
int PacketReader::getTotalCount() const {
	return static_cast<int>(m_newsList.size());
}

// ============================================
// getReadCount: 获取已读取数量
// 返回：已通过 getNextNews 读取的新闻数
// 说明：等于 m_currentIndex
// ============================================
int PacketReader::getReadCount() const {
	return m_currentIndex;
}

// ============================================
// hasNext: 判断是否还有更多新闻
// 返回：有未读新闻返回 true
// 说明：检查 m_currentIndex 是否小于列表大小
// ============================================
bool PacketReader::hasNext() const {
	return m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_newsList.size());
}

// ============================================
// getAllNews: 获取所有新闻
// 返回：NewsPacket 向量，包含所有已加载的新闻
// 说明：返回列表的副本，不影响内部状态
// ============================================
std::vector<NewsPacket> PacketReader::getAllNews() {
	return m_newsList;
}