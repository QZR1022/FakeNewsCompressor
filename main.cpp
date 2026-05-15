#define _CRT_SECURE_NO_WARNINGS

#include "Config.h"
#include "PacketReader.h"
#include "Detector.h"
#include "Tracker.h"
#include "Searcher.h"
#include "ConsoleUI.h"
#include "LZ77.h"
#include "Utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <direct.h> // _mkdir
#include <sys/stat.h> // stat
#include <windows.h>
#include <clocale>

// 检查目录是否存在
static bool dirExists(const std::string& path) {
	struct stat info;
	return (stat(path.c_str(), &info) == 0) && (info.st_mode & S_IFDIR);
}

// 确保目录存在
static bool ensureDir(const std::string& path) {
	if (dirExists(path)) return true;
	return _mkdir(path.c_str()) == 0;
}

// 构造压缩文件路径：archive/<fingerprint>.ncz
static std::string buildArchivePath(size_t fingerprint) {
	std::ostringstream oss;
	oss << ARCHIVE_DIR << fingerprint << COMPRESSED_EXT;
	return oss.str();
}

int main() {
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
	setlocale(LC_ALL, ".UTF-8");

	ConsoleUI ui;
	Detector detector;
	Tracker tracker;
	Searcher searcher;
	LZ77 compressor(LZ77_WINDOW_SIZE, LZ77_LOOKAHEAD_SIZE);

	// 检索上下文
	std::vector<std::string> allContents;
	std::vector<std::string> allTimestamps;
	std::vector<double> allCredibilities;

	// 最近一次监控缓存（给统计模式用）
	bool hasMonitorRun = false;
	int lastTotalNews = 0;
	int lastFakeCount = 0;
	double lastFakeRatio = 0.0;
	double lastAvgRatio = 0.0;
	double lastGlobalRatio = 0.0;
	double lastSavingRate = 0.0;
	int lastExpansionCases = 0;
	size_t lastTotalOriginalBytes = 0;
	size_t lastTotalCompressedBytes = 0;

	bool running = true;

	while (running) {
		int choice = ui.showMainMenu();

		if (choice == 1) {
			// ==============================
			// 监控模式（实时 + 详细）
			// ==============================
			PacketReader reader;
			if (!reader.loadFile(NEWS_FILE)) {
				ui.showError("无法加载新闻文件: " + NEWS_FILE);
				ui.waitForKey();
				continue;
			}

			if (!ensureDir(DATA_DIR)) {
				ui.showError("无法创建/访问数据目录: " + DATA_DIR);
				ui.waitForKey();
				continue;
			}
			if (!ensureDir(ARCHIVE_DIR)) {
				ui.showError("无法创建/访问归档目录: " + ARCHIVE_DIR);
				ui.waitForKey();
				continue;
			}

			allContents.clear();
			allTimestamps.clear();
			allCredibilities.clear();

			int total = reader.getTotalCount();
			int processed = 0;
			int fakeCount = 0;

			double sumRatio = 0.0;
			int ratioCount = 0;

			size_t totalOriginalBytes = 0;
			size_t totalCompressedBytes = 0;
			int expansionCases = 0;

			NewsPacket pkt;
			while (reader.getNextNews(pkt)) {
				processed++;

				DetectionResult dr = detector.detect(pkt.content);

				allContents.push_back(pkt.content);
				allTimestamps.push_back(pkt.timestamp);
				allCredibilities.push_back(dr.credibility);

				bool isFake = (dr.credibility < 50.0);
				if (isFake) {
					fakeCount++;

					size_t fp = tracker.generateFingerprint(pkt.content);
					tracker.recordPropagation(fp, pkt.sourceIP, pkt.timestamp);

					std::vector<char> compressed = compressor.compress(pkt.content);
					std::string outPath = buildArchivePath(fp);

					if (writeBytesToFile(outPath, compressed.data(), compressed.size())) {
						size_t original = pkt.content.size();
						size_t comp = compressed.size();

						if (original > 0) {
							double ratio = compressor.getCompressionRatio(original, comp);
							sumRatio += ratio;
							ratioCount++;

							totalOriginalBytes += original;
							totalCompressedBytes += comp;

							if (comp > original) expansionCases++;
						}
					}
				}

				double avgRatioRealtime = (ratioCount > 0) ? (sumRatio / ratioCount) : 0.0;
				ui.showMonitorPanel(processed, total, fakeCount, avgRatioRealtime);
				ui.showDetectionResult(pkt.content, dr.credibility, isFake);
			}

			searcher.buildIndex(allContents);
			searcher.setNewsContext(allTimestamps, allCredibilities);

			double avgRatio = (ratioCount > 0) ? (sumRatio / ratioCount) : 0.0;
			double fakeRatio = (total > 0) ? ((double)fakeCount / (double)total * 100.0) : 0.0;
			double globalRatio = 0.0;
			double savingRate = 0.0;

			if (totalOriginalBytes > 0) {
				globalRatio = compressor.getCompressionRatio(totalOriginalBytes, totalCompressedBytes);
				savingRate = compressor.getSpaceSavingRate(totalOriginalBytes, totalCompressedBytes);
			}

			std::vector<std::pair<size_t, int>> top3 = tracker.getTopFakeNews(3);
			ui.showStatistics(total, fakeCount, globalRatio, fakeRatio, top3);

			// 监控模式专属详细
			std::cout << "\n============ COMPRESSION DETAILS ============\n";
			std::cout << "Avg Ratio (sample mean): " << avgRatio << "%\n";
			std::cout << "Global Ratio (bytes): " << globalRatio << "%\n";
			std::cout << "Space Saving Rate: " << savingRate << "%\n";
			std::cout << "Original Bytes (total): " << totalOriginalBytes << "\n";
			std::cout << "Compressed Bytes(total): " << totalCompressedBytes << "\n";
			std::cout << "Expansion Cases: " << expansionCases << " / " << fakeCount << "\n";
			std::cout << "Note: Ratio > 100% means expansion on short texts (normal).\n";
			std::cout << "=============================================\n";

			// 缓存给统计模式
			hasMonitorRun = true;
			lastTotalNews = total;
			lastFakeCount = fakeCount;
			lastFakeRatio = fakeRatio;
			lastAvgRatio = avgRatio;
			lastGlobalRatio = globalRatio;
			lastSavingRate = savingRate;
			lastExpansionCases = expansionCases;
			lastTotalOriginalBytes = totalOriginalBytes;
			lastTotalCompressedBytes = totalCompressedBytes;

			ui.showSuccess("监控完成。");
			ui.waitForKey();
		}
		else if (choice == 2) {
			// ==============================
			// 检索模式
			// ==============================
			if (searcher.getDocumentCount() <= 0) {
				ui.showWarning("当前没有可检索数据，请先执行一次监控模式。");
				ui.waitForKey();
				continue;
			}

			ui.clearScreen();
			std::string keyword = ui.getUserInput("请输入关键词（支持单词/短语）: ");
			if (keyword.empty()) {
				ui.showWarning("关键词不能为空。");
				ui.waitForKey();
				continue;
			}

			std::vector<SearchResult> results;
			if (keyword.find(' ') != std::string::npos) {
				results = searcher.searchExact(keyword);
			}
			else {
				results = searcher.search(keyword);
			}

			ui.showSearchResults(results);
			ui.waitForKey();
		}
		else if (choice == 3) {
			// ==============================
			// 统计模式（总览，不重复监控详细）
			// ==============================
			if (!hasMonitorRun) {
				ui.showWarning("暂无统计数据，请先执行一次监控模式。");
				ui.waitForKey();
				continue;
			}

			ui.clearScreen();
			std::cout << "=============== 统计总览（模式3） ===============\n";
			std::cout << "总新闻数: " << lastTotalNews << "\n";
			std::cout << "假新闻数: " << lastFakeCount << "\n";
			std::cout << "假新闻占比: " << lastFakeRatio << "%\n\n";

			std::cout << "【压缩总览】\n";
			std::cout << "- 样本平均压缩率: " << lastAvgRatio << "%\n";
			std::cout << "- 全局字节压缩率: " << lastGlobalRatio << "%";
			if (lastGlobalRatio > 100.0) std::cout << "（膨胀）";
			std::cout << "\n";
			std::cout << "- 空间节省率: " << lastSavingRate << "%\n";
			std::cout << "- 原始总字节: " << lastTotalOriginalBytes << "\n";
			std::cout << "- 压缩后字节: " << lastTotalCompressedBytes << "\n";
			std::cout << "- 膨胀条数: " << lastExpansionCases << " / " << lastFakeCount << "\n\n";

			std::cout << "【传播热点 Top5】\n";
			std::vector<std::pair<size_t, int>> top5 = tracker.getTopFakeNews(5);
			if (top5.empty()) {
				std::cout << "(空)\n";
			}
			else {
				for (size_t i = 0; i < top5.size(); ++i) {
					std::cout << " " << (i + 1)
						<< ". 指纹=" << top5[i].first
						<< "，传播次数=" << top5[i].second << "\n";
				}
			}
			std::cout << "=================================================\n";

			ui.waitForKey();
		}
		else if (choice == 4) {
			running = false;
			ui.showInfo("程序退出。");
		}
	}

	return 0;
}