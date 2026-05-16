#include "Tracker.h"
#include "Utils.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <sstream>

// ============================================
// Tracker.cpp - 假新闻传播追踪器实现文件
// ============================================
// 功能：追踪假新闻的传播路径和来源
// 主要功能：
//   1. 生成新闻内容指纹（哈希值）
//   2. 记录传播来源（IP、时间戳）
//   3. 统计传播次数
//   4. 持久化追踪数据
// 作者：QZR1022
// 版本：1.0
// ============================================

// ============================================
// 构造函数
// 说明：初始化空的追踪记录
// ============================================
Tracker::Tracker() {}

// ============================================
// generateFingerprint: 生成新闻指纹
// 参数：content - 新闻正文
// 返回：64位哈希值
// 说明：
//   - 使用 std::hash<string> 生成哈希值
//   - 相同内容一定生成相同指纹
//   - 用于识别同一假新闻的不同传播实例
//   - 哈希冲突概率极低（64位）
// ============================================
size_t Tracker::generateFingerprint(const std::string& content) {
	std::hash<std::string> hasher;
	return hasher(content);
}

// ============================================
// recordPropagation: 记录一次传播
// 参数：
//   - fingerprint - 新闻指纹
//   - ip - 来源 IP 地址
//   - time - 传播时间戳
// 说明：
//   1. 创建传播记录结构
//   2. 追加到指定指纹的记录列表
//   3. 如果指纹不存在，自动创建新列表
// ============================================
void Tracker::recordPropagation(size_t fingerprint, const std::string& ip, const std::string& time) {
	PropagationRecord rec;
	rec.sourceIP = ip;
	rec.timestamp = time;

	m_propagationMap[fingerprint].push_back(rec);
}

// ============================================
// recordContent: 记录新闻正文供显示使用
// 参数：fingerprint - 新闻指纹，content - 新闻正文
// 说明：存储新闻正文以便后续在统计页面显示新闻内容而非指纹
// ============================================
void Tracker::recordContent(size_t fingerprint, const std::string& content) {
	m_contentMap[fingerprint] = content;
}

// ============================================
// getContent: 获取某条新闻的正文
// 参数：fingerprint - 新闻指纹
// 返回：新闻正文，若不存在返回空字符串
// 说明：用于在统计页面显示新闻内容而非无意义的指纹数字
// ============================================
std::string Tracker::getContent(size_t fingerprint) const {
	std::map<size_t, std::string>::const_iterator it = m_contentMap.find(fingerprint);
	if (it == m_contentMap.end()) {
		return "(内容已丢失)";
	}
	return it->second;
}

// ============================================
// getPropagationCount: 获取某个假新闻的出现次数
// 参数：fingerprint - 新闻指纹
// 返回：该指纹对应的传播记录数量
// 说明：如果指纹不存在，返回 0
// ============================================
int Tracker::getPropagationCount(size_t fingerprint) const {
	std::map<size_t, std::vector<PropagationRecord>>::const_iterator it =
		m_propagationMap.find(fingerprint);
	if (it == m_propagationMap.end()) return 0;  // 指纹不存在
	return (int)it->second.size();  // 返回记录数量
}

// ============================================
// getPropagationHistory: 获取某个假新闻的完整传播记录
// 参数：fingerprint - 新闻指纹
// 返回：传播记录列表（按时间顺序）
// 说明：如果指纹不存在，返回空向量
// ============================================
std::vector<PropagationRecord> Tracker::getPropagationHistory(size_t fingerprint) const {
	std::map<size_t, std::vector<PropagationRecord>>::const_iterator it =
		m_propagationMap.find(fingerprint);
	if (it == m_propagationMap.end()) {
		return std::vector<PropagationRecord>();  // 空列表
	}
	return it->second;
}

// ============================================
// getAllFingerprints: 获取所有被追踪的假新闻指纹
// 返回：所有指纹列表
// 说明：返回 m_propagationMap 的所有键
// ============================================
std::vector<size_t> Tracker::getAllFingerprints() const {
	std::vector<size_t> keys;
	for (std::map<size_t, std::vector<PropagationRecord>>::const_iterator it =
		m_propagationMap.begin(); it != m_propagationMap.end(); ++it) {
		keys.push_back(it->first);
	}
	return keys;
}

// ============================================
// getTopFakeNews: 获取传播次数最多的假新闻（热点）
// 参数：topN - 返回前 N 个
// 返回：pair<指纹, 次数> 列表，按次数降序排序
// 说明：
//   1. 遍历所有指纹，收集 (指纹, 次数) 对
//   2. 按次数降序排序
//   3. 截取前 topN 个
//   4. 次数相同则按指纹数字升序（保持稳定性）
// ============================================
std::vector<std::pair<size_t, int>> Tracker::getTopFakeNews(int topN) const {
	std::vector<std::pair<size_t, int>> list;

	// 参数检查
	if (topN <= 0) return list;

	// 收集所有 (指纹, 次数) 对
	for (std::map<size_t, std::vector<PropagationRecord>>::const_iterator it =
		m_propagationMap.begin(); it != m_propagationMap.end(); ++it) {
		list.push_back(std::make_pair(it->first, (int)it->second.size()));
	}

	// 按次数降序排序，次数相同按指纹升序
	std::sort(list.begin(), list.end(),
		[](const std::pair<size_t, int>& a, const std::pair<size_t, int>& b) {
			if (a.second != b.second) return a.second > b.second;  // 次数降序
			return a.first < b.first;  // 指纹升序
		});

	// 截取前 topN 个
	if ((int)list.size() > topN) {
		list.resize(topN);
	}

	return list;
}

// ============================================
// isTracked: 检查某条新闻是否已被追踪
// 参数：fingerprint - 新闻指纹
// 返回：已被追踪返回 true
// ============================================
bool Tracker::isTracked(size_t fingerprint) const {
	return m_propagationMap.find(fingerprint) != m_propagationMap.end();
}

// ============================================
// saveToFile: 保存追踪数据到文件
// 参数：filePath - 保存路径
// 返回：成功返回 true
// 说明：
//   - 格式：fingerprint|timestamp|IP（每行一条记录）
//   - 同一指纹的多条记录分行保存
//   - 使用文本模式，便于查看和调试
// ============================================
bool Tracker::saveToFile(const std::string& filePath) const {
	// 打开文件（覆盖模式）
	std::ofstream out(filePath.c_str(), std::ios::out | std::ios::trunc);
	if (!out.is_open()) return false;  // 文件打开失败

	// 遍历所有指纹及其传播记录
	for (std::map<size_t, std::vector<PropagationRecord>>::const_iterator it =
		m_propagationMap.begin(); it != m_propagationMap.end(); ++it) {
		size_t fp = it->first;  // 指纹
		const std::vector<PropagationRecord>& vec = it->second;  // 传播记录列表

		// 每条记录一行：指纹|时间戳|IP
		for (size_t i = 0; i < vec.size(); ++i) {
			out << fp << "|" << vec[i].timestamp << "|" << vec[i].sourceIP << "\n";
		}
	}

	out.close();  // 关闭文件
	return true;
}

// ============================================
// loadFromFile: 从文件加载追踪数据
// 参数：filePath - 文件路径
// 返回：成功返回 true
// 说明：
//   1. 清空现有数据（支持重新加载）
//   2. 逐行读取文件
//   3. 解析格式：fingerprint|timestamp|IP
//   4. 跳过格式错误的行
//   5. 加载后可以继续累计传播数据
// ============================================
bool Tracker::loadFromFile(const std::string& filePath) {
	std::ifstream in(filePath.c_str());
	if (!in.is_open()) return false;  // 文件不存在或无法打开

	m_propagationMap.clear();  // 清空现有数据

	std::string line;
	while (std::getline(in, line)) {
		line = trim(line);  // 去除首尾空格
		if (line.empty()) continue;  // 跳过空行

		// 解析格式：fingerprint|timestamp|IP
		std::vector<std::string> parts = splitString(line, '|');
		if (parts.size() < 3) continue;  // 格式错误，跳过

		// 解析指纹（第一个字段）
		std::istringstream iss(parts[0]);
		size_t fp = 0;
		iss >> fp;
		if (!iss) continue;  // 解析失败，跳过

		// 创建传播记录
		PropagationRecord rec;
		rec.timestamp = parts[1];  // 时间戳
		rec.sourceIP = parts[2];   // IP 地址

		// 添加到对应指纹的记录列表
		m_propagationMap[fp].push_back(rec);
	}

	in.close();  // 关闭文件
	return true;
}

// ============================================
// clear: 清空所有追踪数据
// 说明：
//   - 清空传播记录映射
//   - 清空新闻正文映射
//   - 用于重置追踪状态
// ============================================
void Tracker::clear() {
	m_propagationMap.clear();
	m_contentMap.clear();
}