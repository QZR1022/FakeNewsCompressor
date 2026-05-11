#ifndef DETECTOR_H
#define DETECTOR_H

#include <string>
#include <vector>

// 特征提取结果
struct Features {
    int exclamationCount;      // 感叹号数量
    int questionCount;         // 问号数量
    double uppercaseRatio;     // 大写字母比例（0-1）
    int sensitiveWordCount;    // 敏感词命中数量
    double avgSentenceLength;  // 平均句子长度
};

// 检测结果
struct DetectionResult {
    double credibility;        // 可信度（0-100）
    std::string label;         // 标签："真新闻" / "假新闻" / "可疑"
    Features features;         // 提取的特征
};

// 假新闻检测器类
class Detector {
public:
    // 构造函数：加载权重数据
    Detector();

    // 从文本中提取特征
    // 参数：text - 新闻正文
    // 返回：特征结构体
    Features extractFeatures(const std::string& text);

    // 朴素贝叶斯分类
    // 参数：features - 特征数据
    // 返回：假新闻的概率（0-1）
    double naiveBayesPredict(const Features& features);

    // 检测单条新闻
    // 参数：content - 新闻正文
    // 返回：检测结果
    DetectionResult detect(const std::string& content);

    // 批量检测
    // 参数：contents - 多条新闻正文
    // 返回：每条新闻的检测结果
    std::vector<DetectionResult> detectBatch(const std::vector<std::string>& contents);

    // 获取假新闻判定阈值
    double getThreshold() const;

    // 设置假新闻判定阈值（默认50，低于此值为假新闻）
    void setThreshold(double threshold);

private:
    double m_threshold;  // 判定阈值
};

#endif // DETECTOR_H

