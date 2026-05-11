#ifndef TRACKER_H
#define TRACKER_H

#include <string>
#include <vector>
#include <map>
#include <cstddef>

// 单次传播记录
struct PropagationRecord {
    std::string timestamp;   // 出现时间
    std::string sourceIP;    // 来源IP
};

// 传播追踪器类
class Tracker {
public:
    // 构造函数
    Tracker();

    // 生成新闻指纹（哈希值）
    // 参数：content - 新闻正文
    // 返回：64位哈希值
    size_t generateFingerprint(const std::string& content);

    // 记录一次传播
    // 参数：fingerprint - 新闻指纹，ip - 来源IP，time - 时间戳
    void recordPropagation(size_t fingerprint, const std::string& ip, const std::string& time);

    // 获取某个假新闻的出现次数
    // 参数：fingerprint - 新闻指纹
    // 返回：出现次数
    int getPropagationCount(size_t fingerprint) const;

    // 获取某个假新闻的完整传播记录
    // 参数：fingerprint - 新闻指纹
    // 返回：传播记录列表
    std::vector<PropagationRecord> getPropagationHistory(size_t fingerprint) const;

    // 获取所有被追踪的假新闻指纹
    // 返回：所有指纹列表
    std::vector<size_t> getAllFingerprints() const;

    // 获取传播次数最多的假新闻（热点）
    // 参数：topN - 返回前N个
    // 返回：pair<指纹, 次数> 列表，按次数降序
    std::vector<std::pair<size_t, int>> getTopFakeNews(int topN = 5) const;

    // 检查某条新闻是否已被追踪
    // 参数：fingerprint - 新闻指纹
    // 返回：已被追踪返回true
    bool isTracked(size_t fingerprint) const;

    // 保存追踪数据到文件
    // 参数：filePath - 保存路径
    // 返回：成功返回true
    bool saveToFile(const std::string& filePath) const;

    // 从文件加载追踪数据
    // 参数：filePath - 文件路径
    // 返回：成功返回true
    bool loadFromFile(const std::string& filePath);

    // 清空所有追踪数据
    void clear();

private:
    // 存储指纹 -> 传播记录列表
    std::map<size_t, std::vector<PropagationRecord>> m_propagationMap;
};

#endif // TRACKER_H


