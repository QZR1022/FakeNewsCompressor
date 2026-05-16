#ifndef TRACKER_H
#define TRACKER_H

// ============================================
// Tracker.h - 假新闻传播追踪器头文件
// ============================================
// 功能：追踪假新闻的传播路径和来源
// 主要功能：
//   1. 生成新闻内容指纹（哈希值）
//   2. 记录传播来源（IP、 timestamp）
//   3. 统计传播次数
//   4. 持久化追踪数据
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>
#include <vector>
#include <map>
#include <cstddef>

// ============================================
// PropagationRecord - 单次传播记录结构
// 说明：记录一次假新闻传播事件
// ============================================
struct PropagationRecord {
	std::string timestamp;   // 传播时间（格式：YYYY-MM-DD HH:MM:SS）
	std::string sourceIP;    // 来源 IP 地址
};

// ============================================
// Tracker - 假新闻传播追踪器类
// 说明：
//   - 使用内容指纹识别同一假新闻的不同传播实例
//   - 记录每条假新闻的传播来源和时间
//   - 支持按传播次数排序获取热点假新闻
// 数据结构：
//   - m_propagationMap: 指纹 -> 传播记录列表
//   - m_contentMap: 指纹 -> 新闻正文（用于显示）
// ============================================
class Tracker {
public:
	// ============================================
	// 构造函数
	// 说明：初始化空的追踪记录
	// ============================================
	Tracker();

	// ============================================
	// generateFingerprint: 生成新闻指纹
	// 参数：content - 新闻正文
	// 返回：64位哈希值（基于 std::hash）
	// 说明：
	//   - 相同内容生成相同指纹
	//   - 用于识别同一假新闻的不同传播实例
	//   - 使用 std::hash<string> 生成哈希值
	// ============================================
	size_t generateFingerprint(const std::string& content);

	// ============================================
	// recordPropagation: 记录一次传播
	// 参数：
	//   - fingerprint - 新闻指纹
	//   - ip - 来源 IP 地址
	//   - time - 传播时间戳
	// 说明：向指定指纹的传播记录列表追加新记录
	// ============================================
	void recordPropagation(size_t fingerprint, const std::string& ip, const std::string& time);

	// ============================================
	// recordContent: 记录新闻正文（供显示用）
	// 参数：
	//   - fingerprint - 新闻指纹
	//   - content - 新闻正文
	// 说明：存储新闻正文以便后续显示，而非显示无意义的指纹数字
	// ============================================
	void recordContent(size_t fingerprint, const std::string& content);

	// ============================================
	// getContent: 获取某条新闻的正文
	// 参数：fingerprint - 新闻指纹
	// 返回：新闻正文，若不存在返回 "(内容已丢失)"
	// 说明：用于在统计页面显示新闻内容而非指纹
	// ============================================
	std::string getContent(size_t fingerprint) const;

	// ============================================
	// getPropagationCount: 获取传播次数
	// 参数：fingerprint - 新闻指纹
	// 返回：该指纹对应的传播记录数量
	// ============================================
	int getPropagationCount(size_t fingerprint) const;

	// ============================================
	// getPropagationHistory: 获取完整传播记录
	// 参数：fingerprint - 新闻指纹
	// 返回：传播记录列表（按时间顺序）
	// ============================================
	std::vector<PropagationRecord> getPropagationHistory(size_t fingerprint) const;

	// ============================================
	// getAllFingerprints: 获取所有被追踪的指纹
	// 返回：指纹向量
	// ============================================
	std::vector<size_t> getAllFingerprints() const;

	// ============================================
	// getTopFakeNews: 获取传播热点
	// 参数：topN - 返回前 N 个
	// 返回：pair<指纹, 次数> 向量，按次数降序排序
	// 说明：用于显示传播最广的假新闻
	// ============================================
	std::vector<std::pair<size_t, int>> getTopFakeNews(int topN = 5) const;

	// ============================================
	// isTracked: 检查是否已被追踪
	// 参数：fingerprint - 新闻指纹
	// 返回：已被追踪返回 true
	// ============================================
	bool isTracked(size_t fingerprint) const;

	// ============================================
	// saveToFile: 保存追踪数据到文件
	// 参数：filePath - 文件路径
	// 返回：成功返回 true
	// 说明：
	//   - 格式：fingerprint|timestamp|IP（每行一条记录）
	//   - 同一指纹的多条记录分行保存
	// ============================================
	bool saveToFile(const std::string& filePath) const;

	// ============================================
	// loadFromFile: 从文件加载追踪数据
	// 参数：filePath - 文件路径
	// 返回：成功返回 true
	// 说明：加载后可以继续累计传播数据
	// ============================================
	bool loadFromFile(const std::string& filePath);

	// ============================================
	// clear: 清空所有追踪数据
	// 说明：用于重置追踪状态
	// ============================================
	void clear();

private:
	// ============================================
	// 私有成员变量
	// ============================================
	std::map<size_t, std::vector<PropagationRecord>> m_propagationMap;  // 指纹 -> 传播记录
	std::map<size_t, std::string> m_contentMap;                          // 指纹 -> 新闻正文
};

#endif // TRACKER_H