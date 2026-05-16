#include "Detector.h"
#include "Weights.h"
#include "Utils.h"

#include <cctype>
#include <cmath>
#include <string>
#include <vector>

// ============================================
// Detector.cpp - 假新闻检测器实现文件
// ============================================
// 功能：基于朴素贝叶斯分类器检测假新闻
// 算法流程：
//   1. 特征提取：感叹号、问号、大写比例、敏感词、句长
//   2. 归一化：将特征值缩放到 [0, 1]
//   3. 加权求和：score = LOG_PRIOR + sum(weight * normalized_value)
//   4. Sigmoid 输出：pFake = 1 / (1 + exp(diff))
// 支持：中英文混合检测（UTF-8 处理）
// 作者：QZR1022
// 版本：1.0
// ============================================

// ============================================
// 构造函数
// 说明：初始化假新闻阈值（50%）
// 阈值用于判断新闻是否被标记为假新闻
// ============================================
Detector::Detector() {
	m_threshold = 50.0;
}

// ============================================
// extractFeatures: 特征提取函数
// 参数：text - 新闻正文（UTF-8 编码）
// 返回：Features 结构，包含 5 维特征
// 说明：
//   - 中文字符在 UTF-8 中占用 3 字节（0xE?开头）
//   - 英文感叹号(!) ASCII 33，UTF-8 单字节
//   - 中文感叹号(！) UTF-8: EF BC 81
//   - 英文问号(?) ASCII 63，UTF-8 单字节
//   - 中文问号(？) UTF-8: EF BC 9F
//   - 中文句号(。) UTF-8: E3 80 82
// ============================================
Features Detector::extractFeatures(const std::string& text) {
	Features f;
	f.exclamationCount = 0;
	f.questionCount = 0;
	f.uppercaseRatio = 0.0;
	f.sensitiveWordCount = 0;
	f.avgSentenceLength = 0.0;

	// 空文本直接返回
	if (text.empty()) {
		return f;
	}

	// ============================================
	// 特征1：统计感叹号数量
	// 说明：英文(!) 和中文(！) 都要统计
	// ============================================
	for (size_t i = 0; i < text.size(); ++i) {
		unsigned char ch = static_cast<unsigned char>(text[i]);

		// 英文感叹号：ASCII 33
		if (ch == '!') f.exclamationCount++;

		// 英文问号：ASCII 63
		if (ch == '?') f.questionCount++;

		// 中文感叹号(！)：UTF-8 EF BC 81
		if (ch == 0xEF && i + 2 < text.size()) {
			unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
			unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
			if (b1 == 0xBC && b2 == 0x81) f.exclamationCount++;

			// 中文问号(？)：UTF-8 EF BC 9F
			if (b1 == 0xBC && b2 == 0x9F) f.questionCount++;
		}
	}

	// ============================================
	// 特征2：计算大写字母比例（仅英文）
	// 说明：只统计英文字母中的大写比例
	// ============================================
	int alphaCount = 0;   // 英文字母总数
	int upperCount = 0;    // 大写字母数量
	for (size_t i = 0; i < text.size(); ++i) {
		unsigned char ch = static_cast<unsigned char>(text[i]);
		if (std::isalpha(ch)) {  // 判断是否为英文字母
			alphaCount++;
			if (std::isupper(ch)) {  // 判断是否为大写
				upperCount++;
			}
		}
	}
	if (alphaCount > 0) {
		f.uppercaseRatio = static_cast<double>(upperCount) / static_cast<double>(alphaCount);
	}

	// ============================================
	// 特征3：统计敏感词命中数量
	// 说明：
	//   - 将文本转小写后进行匹配
	//   - 允许重叠匹配（"不不"中"不"出现2次）
	//   - 敏感词列表定义在 Weights.h 的 SENSITIVE_WORDS
	// ============================================
	std::string textLower = toLower(text);  // 文本小写化
	for (size_t i = 0; i < SENSITIVE_WORDS.size(); ++i) {
		const std::string& rawWord = SENSITIVE_WORDS[i].first;  // 敏感词
		if (rawWord.empty()) continue;

		std::string wordLower = toLower(rawWord);  // 敏感词小写化
		size_t pos = 0;
		while (true) {
			pos = textLower.find(wordLower, pos);  // 在文本中查找
			if (pos == std::string::npos) break;  // 未找到，结束
			f.sensitiveWordCount++;  // 命中次数 +1
			pos += wordLower.size();  // 移动到下次搜索位置（允许重叠）
		}
	}

	// ============================================
	// 特征4：计算平均句子长度
	// 说明：
	//   - 句子结束标志：英文(.!?) 和中文(。！？)
	//   - avgSentenceLength = 总字符数 / 句子数
	// ============================================
	int sentenceCount = 0;  // 句子数量
	for (size_t i = 0; i < text.size(); ++i) {
		unsigned char ch = static_cast<unsigned char>(text[i]);

		// 英文句末标志：. ! ?
		if (ch == '.' || ch == '!' || ch == '?') {
			sentenceCount++;
		}

		// 中文句号(。)：UTF-8 E3 80 82
		if (ch == 0xE3 && i + 2 < text.size()) {
			unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
			unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
			if (b1 == 0x80 && b2 == 0x82) sentenceCount++;
		}

		// 中文感叹号/问号(！？)：UTF-8 EF BC 81/9F
		if (ch == 0xEF && i + 2 < text.size()) {
			unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
			unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
			if (b1 == 0xBC && (b2 == 0x81 || b2 == 0x9F)) sentenceCount++;
		}
	}
	if (sentenceCount <= 0) sentenceCount = 1;  // 避免除零

	f.avgSentenceLength = static_cast<double>(text.size()) / static_cast<double>(sentenceCount);

	return f;
}

// ============================================
// naiveBayesPredict: 朴素贝叶斯预测
// 参数：features - 新闻特征
// 返回：假新闻概率（0-1）
// 算法：
//   1. 归一化：每个特征值 / 最大值，限制在 [0, 1]
//   2. 计算 fake_score 和 real_score：
//      score_fake = LOG_PRIOR_FAKE + sum(FEATURE_WEIGHTS[i] * xi)
//      score_real = LOG_PRIOR_REAL + sum((1-FEATURE_WEIGHTS[i]) * xi)
//   3. 差值裁剪：限制在 [-20, 20] 防止 exp 溢出
//   4. Sigmoid 输出：pFake = 1 / (1 + exp(score_real - score_fake))
// 说明：
//   - LOG_PRIOR_FAKE/REAL 是先验概率的对数
//   - FEATURE_WEIGHTS[i] 表示该特征对假新闻的贡献度
// ============================================
double Detector::naiveBayesPredict(const Features& features) {
	// 归一化特征值到 [0, 1]
	// 除以 FEATURE_MAX_VALUES 中的最大值，并限制不超过 1.0
	double x0 = static_cast<double>(features.exclamationCount) / FEATURE_MAX_VALUES[0];
	double x1 = static_cast<double>(features.questionCount) / FEATURE_MAX_VALUES[1];
	double x2 = features.uppercaseRatio / FEATURE_MAX_VALUES[2];
	double x3 = static_cast<double>(features.sensitiveWordCount) / FEATURE_MAX_VALUES[3];
	double x4 = features.avgSentenceLength / FEATURE_MAX_VALUES[4];

	// 限制在 [0, 1] 范围内
	if (x0 > 1.0) x0 = 1.0;
	if (x1 > 1.0) x1 = 1.0;
	if (x2 > 1.0) x2 = 1.0;
	if (x3 > 1.0) x3 = 1.0;
	if (x4 > 1.0) x4 = 1.0;

	// 计算假新闻得分：LOG_PRIOR_FAKE + sum(weight * normalized_value)
	// LOG_PRIOR_FAKE = ln(0.45) ≈ -0.798507
	// FEATURE_WEIGHTS[0-4] = {0.28, 0.24, 0.32, 0.58, 0.18}
	double score_fake = LOG_PRIOR_FAKE;
	score_fake += FEATURE_WEIGHTS[0] * x0;  // 感叹号权重
	score_fake += FEATURE_WEIGHTS[1] * x1;  // 问号权重
	score_fake += FEATURE_WEIGHTS[2] * x2;  // 大写比例权重
	score_fake += FEATURE_WEIGHTS[3] * x3;  // 敏感词权重
	score_fake += FEATURE_WEIGHTS[4] * x4;  // 句长权重

	// 计算真新闻得分：LOG_PRIOR_REAL + sum((1-weight) * normalized_value)
	// LOG_PRIOR_REAL = ln(0.55) ≈ -0.597837
	// 1 - FEATURE_WEIGHTS[i] 表示对真新闻的贡献
	double score_real = LOG_PRIOR_REAL;
	score_real += (1.0 - FEATURE_WEIGHTS[0]) * x0;
	score_real += (1.0 - FEATURE_WEIGHTS[1]) * x1;
	score_real += (1.0 - FEATURE_WEIGHTS[2]) * x2;
	score_real += (1.0 - FEATURE_WEIGHTS[3]) * x3;
	score_real += (1.0 - FEATURE_WEIGHTS[4]) * x4;

	// 计算差值并裁剪，防止 exp 溢出
	double diff = score_real - score_fake;
	if (diff > 20.0) diff = 20.0;   // 上限
	if (diff < -20.0) diff = -20.0;  // 下限

	// Sigmoid 函数：pFake = 1 / (1 + exp(diff))
	double pFake = 1.0 / (1.0 + std::exp(diff));

	// 限制输出范围
	if (pFake < 0.0) pFake = 0.0;
	if (pFake > 1.0) pFake = 1.0;

	return pFake;
}

// ============================================
// detect: 检测单条新闻
// 参数：content - 新闻正文
// 返回：DetectionResult 结构
// 说明：
//   - credibility: 假新闻可信度（0-100）
//   - label: 根据可信度判断的标签
//     * credibility < 40：真新闻
//     * credibility >= 40 && credibility <= 60：可疑
//     * credibility > 60：假新闻
// ============================================
DetectionResult Detector::detect(const std::string& content) {
	DetectionResult r;

	// 步骤1：特征提取
	r.features = extractFeatures(content);

	// 步骤2：朴素贝叶斯预测
	double pFake = naiveBayesPredict(r.features);

	// 步骤3：转换为可信度（0-100）
	// 可信度定义：值越高，越可能是假新闻
	r.credibility = pFake * 100.0;

	// 步骤4：判断标签
	// 阈值 50% 是判断假新闻的分界线
	if (r.credibility < 40.0) {
		r.label = "真新闻";
	}
	else if (r.credibility > 60.0) {
		r.label = "假新闻";
	}
	else {
		r.label = "可疑";
	}

	return r;
}

// ============================================
// detectBatch: 批量检测新闻
// 参数：contents - 新闻正文向量
// 返回：DetectionResult 向量
// 说明：对每条新闻调用 detect()，收集结果
// ============================================
std::vector<DetectionResult> Detector::detectBatch(const std::vector<std::string>& contents) {
	std::vector<DetectionResult> out;
	out.reserve(contents.size());  // 预分配容量，避免多次扩容

	for (size_t i = 0; i < contents.size(); ++i) {
		out.push_back(detect(contents[i]));
	}

	return out;
}

// ============================================
// getThreshold: 获取当前阈值
// 返回：假新闻判断阈值（默认 50%）
// ============================================
double Detector::getThreshold() const {
	return m_threshold;
}

// ============================================
// setThreshold: 设置阈值
// 参数：threshold - 新的阈值（限制在 0-100）
// 说明：阈值的调整会影响 label 的判断结果
// ============================================
void Detector::setThreshold(double threshold) {
	if (threshold < 0.0) threshold = 0.0;
	if (threshold > 100.0) threshold = 100.0;
	m_threshold = threshold;
}