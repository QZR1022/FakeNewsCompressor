#include "Detector.h"
#include "Weights.h"
#include "Utils.h"

#include <cctype>
#include <cmath>
#include <string>
#include <vector>

Detector::Detector() {
    m_threshold = 50.0;
}

Features Detector::extractFeatures(const std::string& text) {
    Features f;
    f.exclamationCount = 0;
    f.questionCount = 0;
    f.uppercaseRatio = 0.0;
    f.sensitiveWordCount = 0;
    f.avgSentenceLength = 0.0;

    if (text.empty()) {
        return f;
    }

    // 1) 感叹号、问号数量（英文 + 中文）
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(text[i]);

        // 英文
        if (ch == '!') f.exclamationCount++;
        if (ch == '?') f.questionCount++;

        // 中文 “！”“？” UTF-8: EF BC 81 / EF BC 9F
        if (ch == 0xEF && i + 2 < text.size()) {
            unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
            unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
            if (b1 == 0xBC && b2 == 0x81) f.exclamationCount++;
            if (b1 == 0xBC && b2 == 0x9F) f.questionCount++;
        }
    }

    // 2) 大写字母比例（仅英文）
    int alphaCount = 0;
    int upperCount = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        if (std::isalpha(ch)) {
            alphaCount++;
            if (std::isupper(ch)) {
                upperCount++;
            }
        }
    }
    if (alphaCount > 0) {
        f.uppercaseRatio = static_cast<double>(upperCount) / static_cast<double>(alphaCount);
    }

    // 3) 敏感词命中数（转小写后比较）
    std::string textLower = toLower(text);
    for (size_t i = 0; i < SENSITIVE_WORDS.size(); ++i) {
        const std::string& rawWord = SENSITIVE_WORDS[i].first;
        if (rawWord.empty()) continue;

        std::string wordLower = toLower(rawWord);
        size_t pos = 0;
        while (true) {
            pos = textLower.find(wordLower, pos);
            if (pos == std::string::npos) break;
            f.sensitiveWordCount++;
            pos += wordLower.size();
        }
    }

    // 4) 平均句子长度（按 . ! ? 以及中文 。！？分句）
    int sentenceCount = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(text[i]);

        // 英文句末
        if (ch == '.' || ch == '!' || ch == '?') {
            sentenceCount++;
        }

        // 中文句号 "。" UTF-8: E3 80 82
        if (ch == 0xE3 && i + 2 < text.size()) {
            unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
            unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
            if (b1 == 0x80 && b2 == 0x82) sentenceCount++;
        }

        // 中文 "！" "？"
        if (ch == 0xEF && i + 2 < text.size()) {
            unsigned char b1 = static_cast<unsigned char>(text[i + 1]);
            unsigned char b2 = static_cast<unsigned char>(text[i + 2]);
            if (b1 == 0xBC && (b2 == 0x81 || b2 == 0x9F)) sentenceCount++;
        }
    }
    if (sentenceCount <= 0) sentenceCount = 1;

    f.avgSentenceLength = static_cast<double>(text.size()) / static_cast<double>(sentenceCount);

    return f;
}

double Detector::naiveBayesPredict(const Features& features) {
    // 归一化到 [0,1]
    double x0 = static_cast<double>(features.exclamationCount) / FEATURE_MAX_VALUES[0];
    double x1 = static_cast<double>(features.questionCount) / FEATURE_MAX_VALUES[1];
    double x2 = features.uppercaseRatio / FEATURE_MAX_VALUES[2];
    double x3 = static_cast<double>(features.sensitiveWordCount) / FEATURE_MAX_VALUES[3];
    double x4 = features.avgSentenceLength / FEATURE_MAX_VALUES[4];

    if (x0 > 1.0) x0 = 1.0;
    if (x1 > 1.0) x1 = 1.0;
    if (x2 > 1.0) x2 = 1.0;
    if (x3 > 1.0) x3 = 1.0;
    if (x4 > 1.0) x4 = 1.0;

    // 目标公式：
    // score_fake = LOG_PRIOR_FAKE + sum(xi * wi)
    // score_real = LOG_PRIOR_REAL + sum(xi * (1 - wi))
    // pFake = 1 / (1 + exp(score_real - score_fake))
    double score_fake = LOG_PRIOR_FAKE;
    double score_real = LOG_PRIOR_REAL;

    score_fake += FEATURE_WEIGHTS[0] * x0;
    score_fake += FEATURE_WEIGHTS[1] * x1;
    score_fake += FEATURE_WEIGHTS[2] * x2;
    score_fake += FEATURE_WEIGHTS[3] * x3;
    score_fake += FEATURE_WEIGHTS[4] * x4;

    score_real += (1.0 - FEATURE_WEIGHTS[0]) * x0;
    score_real += (1.0 - FEATURE_WEIGHTS[1]) * x1;
    score_real += (1.0 - FEATURE_WEIGHTS[2]) * x2;
    score_real += (1.0 - FEATURE_WEIGHTS[3]) * x3;
    score_real += (1.0 - FEATURE_WEIGHTS[4]) * x4;

    double diff = score_real - score_fake;
    if (diff > 20.0) diff = 20.0;
    if (diff < -20.0) diff = -20.0;

    double pFake = 1.0 / (1.0 + std::exp(diff));
    if (pFake < 0.0) pFake = 0.0;
    if (pFake > 1.0) pFake = 1.0;
    return pFake;
}

DetectionResult Detector::detect(const std::string& content) {
    DetectionResult r;
    r.features = extractFeatures(content);

    double pFake = naiveBayesPredict(r.features);

    // 你的目标定义：credibility = 假新闻概率 * 100
    r.credibility = pFake * 100.0;

    // 标签
    if (r.credibility < 40.0) {
        r.label = "假新闻";
    }
    else if (r.credibility > 60.0) {
        r.label = "真新闻";
    }
    else {
        r.label = "可疑";
    }

    return r;
}

std::vector<DetectionResult> Detector::detectBatch(const std::vector<std::string>& contents) {
    std::vector<DetectionResult> out;
    out.reserve(contents.size());

    for (size_t i = 0; i < contents.size(); ++i) {
        out.push_back(detect(contents[i]));
    }

    return out;
}

double Detector::getThreshold() const {
    return m_threshold;
}

void Detector::setThreshold(double threshold) {
    if (threshold < 0.0) threshold = 0.0;
    if (threshold > 100.0) threshold = 100.0;
    m_threshold = threshold;
}