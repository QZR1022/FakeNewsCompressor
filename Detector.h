#ifndef DETECTOR_H
#define DETECTOR_H

// ============================================
// Detector.h - 假新闻检测器头文件
// ============================================
// 功能：基于朴素贝叶斯分类器检测假新闻
// 算法：5 维特征提取 + 归一化 + 加权求和 + Sigmoid 输出
// 特点：
//   - 特征：感叹号数量、问号数量、大写字母比例、敏感词命中数、平均句长
//   - 支持中英文检测（UTF-8 处理）
//   - 权重可配置（Weights.h）
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>
#include <vector>

// ============================================
// Features - 新闻特征结构
// 说明：提取的新闻文本特征，用于分类器输入
// ============================================
struct Features {
	int exclamationCount;   // 感叹号数量（英文! + 中文！）
	double uppercaseRatio;   // 大写字母比例（英文）
	int questionCount;      // 问号数量（英文? + 中文？）
	int sensitiveWordCount; // 敏感词命中数量
	double avgSentenceLength; // 平均句子长度
};

// ============================================
// DetectionResult - 检测结果结构
// ============================================
struct DetectionResult {
	Features features;     // 提取的特征
	double credibility;    // 可信度（0-100），值越高越可能是假新闻
	std::string label;     // 标签："假新闻"、"真新闻"、"可疑"
};

// ============================================
// Detector - 假新闻检测器类
// 说明：
//   - 5 维特征提取（中英文支持）
//   - 归一化到 [0,1]
//   - 朴素贝叶斯加权求和
//   - Sigmoid 输出概率
// ============================================
class Detector {
public:
	// ============================================
	// 构造函数
	// 说明：初始化阈值（50%）
	// ============================================
	Detector();

	// ============================================
	// extractFeatures: 特征提取
	// 参数：text - 新闻正文（UTF-8）
	// 返回：Features 结构
	// 提取的特征：
	//   1. exclamationCount - 感叹号数量（英文! + 中文！）
	//   2. questionCount - 问号数量（英文? + 中文？）
	//   3. uppercaseRatio - 大写字母比例（仅英文）
	//   4. sensitiveWordCount - 敏感词命中数量
	//   5. avgSentenceLength - 平均句子长度
	// 说明：UTF-8 中文字符占用 3 字节，需要特殊处理
	// ============================================
	Features extractFeatures(const std::string& text);

	// ============================================
	// naiveBayesPredict: 朴素贝叶斯预测
	// 参数：features - 新闻特征
	// 返回：假新闻概率（0-1）
	// 算法：
	//   1. 归一化特征值到 [0,1]
	//   2. 计算 fake_score = LOG_PRIOR_FAKE + sum(wi * xi)
	//   3. 计算 real_score = LOG_PRIOR_REAL + sum((1-wi) * xi)
	//   4. pFake = 1 / (1 + exp(real_score - fake_score))
	// ============================================
	double naiveBayesPredict(const Features& features);

	// ============================================
	// detect: 检测单条新闻
	// 参数：content - 新闻正文
	// 返回：DetectionResult 结构
	// 说明：
	//   - credibility: 假新闻可信度（0-100）
	//   - label: 根据阈值判断的标签
	// ============================================
	DetectionResult detect(const std::string& content);

	// ============================================
	// detectBatch: 批量检测新闻
	// 参数：contents - 新闻正文向量
	// 返回：DetectionResult 向量
	// ============================================
	std::vector<DetectionResult> detectBatch(const std::vector<std::string>& contents);

	// ============================================
	// 阈值控制函数
	// ============================================
	double getThreshold() const;   // 获取当前阈值
	void setThreshold(double threshold);  // 设置阈值

private:
	double m_threshold;  // 假新闻阈值（默认 50）
};

#endif // DETECTOR_H