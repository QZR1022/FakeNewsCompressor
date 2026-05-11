#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <iomanip>

#include "Config.h"
#include "Utils.h"
#include "PacketReader.h"
#include "Detector.h"
#include "Tracker.h"
#include "LZ77.h"
#include "Searcher.h"
#include "ConsoleUI.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {

#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	ConsoleUI ui;
	PacketReader reader;
	Detector detector;
	Tracker tracker;
	LZ77 lz77(LZ77_WINDOW_SIZE, LZ77_LOOKAHEAD_SIZE);
	Searcher searcher;

	detector.setThreshold(40.0); // 先给一个更稳的阈值，后续可调

	// 2) 准备目录
	createDirectoryIfNotExists(DATA_DIR);
	createDirectoryIfNotExists(ARCHIVE_DIR);

	ui.showSplashScreen();

	// 3) 读取新闻文件
	if (!reader.loadFile(NEWS_FILE)) {
		ui.showError("Failed to load news file: " + NEWS_FILE);
		ui.showInfo("Please check file path and format: timestamp|ip|content");
		ui.waitForKey();
		return 1;
	}


	std::vector<NewsPacket> allNews = reader.getAllNews();

	if ((int)allNews.size() > MAX_NEWS_COUNT) {
		allNews.resize(MAX_NEWS_COUNT);
		ui.showWarning("Input too large, truncated to MAX_NEWS_COUNT = " + std::to_string(MAX_NEWS_COUNT));
	}

	int totalNews = (int)allNews.size();


	if (totalNews <= 0) {
		ui.showWarning("No valid news data found.");
		ui.waitForKey();
		return 0;
	}

	// 4) 预处理容器（给检索和统计用）
	std::vector<std::string> contents;
	std::vector<std::string> timestamps;
	std::vector<double> credibilities;
	contents.reserve(allNews.size());
	timestamps.reserve(allNews.size());
	credibilities.reserve(allNews.size());

	int fakeCount = 0;
	double ratioSum = 0.0;
	int ratioCount = 0;

	// 5) 主菜单循环
	while (true) {
		int choice = ui.showMainMenu();

		if (choice == 1) {
			// 监控流程：读取->检测->可疑压缩->追踪
			fakeCount = 0;
			ratioSum = 0.0;
			ratioCount = 0;

			contents.clear();
			timestamps.clear();
			credibilities.clear();

			int archiveSuccess = 0;
			int archiveFail = 0;

			for (int i = 0; i < totalNews; ++i) {
				const NewsPacket& pkt = allNews[i];
				DetectionResult dr = detector.detect(pkt.content);

				contents.push_back(pkt.content);
				timestamps.push_back(pkt.timestamp);
				credibilities.push_back(dr.credibility);

				bool isFake = (dr.label == "FAKE" );
				if (isFake) {
					fakeCount++;

					// 生成指纹并记录传播
					size_t fp = tracker.generateFingerprint(pkt.content);
					tracker.recordPropagation(fp, pkt.sourceIP, pkt.timestamp);

					// 对假新闻进行压缩归档
					std::vector<char> compressed = lz77.compress(pkt.content);

					size_t o = 0, z = 0, t = 0;
					lz77.getLastStats(o, z, t);
					if (o > 0) {
						double ratio = lz77.getCompressionRatio(o, z);
						ratioSum += ratio;
						ratioCount++;
					}

					// 归档文件名：fingerprint.ncz
					std::string outPath = ARCHIVE_DIR + std::to_string(fp) + COMPRESSED_EXT;
					if (!compressed.empty()) {
						bool ok = writeBytesToFile(outPath, compressed.data(), compressed.size());
						if (ok) {
							archiveSuccess++;
						}
						else {
							archiveFail++;
							ui.showWarning("Archive write failed: " + outPath);
						}
					}
				}

				double avgRatio = (ratioCount > 0) ? (ratioSum / ratioCount) : 0.0;
				ui.showMonitorPanel(i + 1, totalNews, fakeCount, avgRatio);
			}

			// 构建检索索引
			searcher.buildIndex(contents);
			searcher.setNewsContext(timestamps, credibilities);

			ui.showSuccess("Monitoring completed.");
			ui.showInfo("Total: " + std::to_string(totalNews) +
				", Fake: " + std::to_string(fakeCount));
			ui.showInfo("Archive write success: " + std::to_string(archiveSuccess) +
				", fail: " + std::to_string(archiveFail));
			ui.waitForKey();
		}
		else if (choice == 2) {
			// 检索
			if (contents.empty()) {
				ui.showWarning("Please run Monitoring first (Menu 1).");
				ui.waitForKey();
				continue;
			}

			ui.clearScreen();
			ui.showInfo("Search mode:");
			ui.showInfo("1) Exact phrase search (recommended for Chinese)");
			ui.showInfo("2) Keyword search (better for English tokens)");
			std::cout << "Choose mode (1-2): ";
			int mode = ui.getUserChoice(1, 2);

			std::string key = ui.getUserInput("Input keyword/phrase: ");
			key = trim(key);

			if (key.empty()) {
				ui.showWarning("Keyword/phrase is empty.");
				ui.waitForKey();
				continue;
			}

			std::vector<SearchResult> results;

			if (mode == 1) {
				results = searcher.searchExact(key);
			}
			else {
				results = searcher.search(key);
			}

			ui.showSearchResults(results);
			ui.waitForKey();
		}
		else if (choice == 3) {
			// 统计
			double avgCompressionRatio = (ratioCount > 0) ? (ratioSum / ratioCount) : 0.0;

			double fakeRatio = 0.0;
			if (totalNews > 0) {
				fakeRatio = (double)fakeCount * 100.0 / (double)totalNews;
			}

			std::vector<std::pair<size_t, int>> topList = tracker.getTopFakeNews(5);
			ui.showStatistics(totalNews, fakeCount, avgCompressionRatio, fakeRatio, topList);
			ui.waitForKey();
		}
		else if (choice == 4) {
			ui.showInfo("Bye.");
			break;
		}
	}

	return 0;
}