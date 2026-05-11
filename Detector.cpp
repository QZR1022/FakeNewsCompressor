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

	size_t i = 0;

	// punctuation count
	for (i = 0; i < text.size(); ++i) {
		char ch = text[i];
		if (ch == '!') f.exclamationCount++;
		if (ch == '?') f.questionCount++;
	}

	// uppercase ratio (English letters only)
	int alphaCount = 0;
	int upperCount = 0;
	for (i = 0; i < text.size(); ++i) {
		unsigned char ch = (unsigned char)text[i];
		if (std::isalpha(ch)) {
			alphaCount++;
			if (std::isupper(ch)) {
				upperCount++;
			}
		}
	}

	if (alphaCount > 0) {
		f.uppercaseRatio = (double)upperCount / (double)alphaCount;
	}
	else {
		f.uppercaseRatio = 0.0;
	}

	// sensitive word hit count
	for (i = 0; i < SENSITIVE_WORDS.size(); ++i) {
		const std::string& word = SENSITIVE_WORDS[i].first;
		if (word.empty()) continue;

		size_t pos = 0;
		while (true) {
			pos = text.find(word, pos);
			if (pos == std::string::npos) break;
			f.sensitiveWordCount++;
			pos += word.size();
		}
	}

	// average sentence length
	int sentenceCount = 0;
	for (i = 0; i < text.size(); ++i) {
		char ch = text[i];
		if (ch == '.' || ch == '!' || ch == '?') {
			sentenceCount++;
		}
	}
	if (sentenceCount <= 0) sentenceCount = 1;

	f.avgSentenceLength = (double)text.size() / (double)sentenceCount;
	return f;
}

double Detector::naiveBayesPredict(const Features& features) {
	double x0 = (double)features.exclamationCount / FEATURE_MAX_VALUES[0];
	double x1 = (double)features.questionCount / FEATURE_MAX_VALUES[1];
	double x2 = features.uppercaseRatio / FEATURE_MAX_VALUES[2];
	double x3 = (double)features.sensitiveWordCount / FEATURE_MAX_VALUES[3];
	double x4 = features.avgSentenceLength / FEATURE_MAX_VALUES[4];

	if (x0 > 1.0) x0 = 1.0;
	if (x1 > 1.0) x1 = 1.0;
	if (x2 > 1.0) x2 = 1.0;
	if (x3 > 1.0) x3 = 1.0;
	if (x4 > 1.0) x4 = 1.0;

	double score = 0.0;
	score += (LOG_PRIOR_FAKE - LOG_PRIOR_REAL);
	score += FEATURE_WEIGHTS[0] * x0;
	score += FEATURE_WEIGHTS[1] * x1;
	score += FEATURE_WEIGHTS[2] * x2;
	score += FEATURE_WEIGHTS[3] * x3;
	score += FEATURE_WEIGHTS[4] * x4;

	score += (double)features.sensitiveWordCount * 0.05;

	if (score > 20.0) score = 20.0;
	if (score < -20.0) score = -20.0;

	double pFake = 1.0 / (1.0 + std::exp(-score));
	if (pFake < 0.0) pFake = 0.0;
	if (pFake > 1.0) pFake = 1.0;
	return pFake;
}

DetectionResult Detector::detect(const std::string& content) {
	DetectionResult r;
	r.features = extractFeatures(content);

	double pFake = naiveBayesPredict(r.features);
	r.credibility = (1.0 - pFake) * 100.0;

	if (r.credibility < m_threshold) {
		r.label = "FAKE";
	}
	else if (r.credibility < m_threshold + 15.0) {
		r.label = "SUSPICIOUS";
	}
	else {
		r.label = "REAL";
	}

	return r;
}

std::vector<DetectionResult> Detector::detectBatch(const std::vector<std::string>& contents) {
	std::vector<DetectionResult> out;
	size_t i = 0;
	for (i = 0; i < contents.size(); ++i) {
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