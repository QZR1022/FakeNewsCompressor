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

// 检查并初始化示例数据
static void ensureSampleData() {
	// 确保目录存在
	ensureDir(DATA_DIR);
	ensureDir(ARCHIVE_DIR);

	// 如果 news.txt 不存在，创建示例数据
	if (!fileExists(NEWS_FILE)) {
		const char* sampleNews =
			"2026-05-14 10:00:01|192.168.1.10|震惊！内部消息曝光，某地自来水含致癌物，赶紧告诉家人！\n"
			"2026-05-14 10:01:12|192.168.1.11|学校教务处发布通知：下周三下午进行期中教学检查，请各班按时到场。\n"
			"2026-05-14 10:02:25|192.168.1.12|传疯了！不转不是中国人，今晚12点后手机将被强制收费！\n"
			"2026-05-14 10:03:08|192.168.1.13|市气象台发布天气预报：明日多云转小雨，气温22到28度。\n"
			"2026-05-14 10:04:44|192.168.1.14|删除前速看！某知名食品被曝全部下架，太可怕了！\n"
			"2026-05-14 10:05:51|192.168.1.15|图书馆公告：本周末三楼自习区临时维护，开放时间调整为8:00-18:00。\n"
			"2026-05-14 10:07:03|192.168.1.16|紧急提醒！！！出大事了！！这个链接不点后悔一辈子！！！\n"
			"2026-05-14 10:08:27|192.168.1.17|医院官方辟谣：网传“本院停诊”消息不实，请以公众号公告为准。\n"
			"2026-05-14 10:09:49|192.168.1.18|惊人发现：每天喝这杯水，三天清除体内所有毒素，专家都不敢说！\n"
			"2026-05-14 10:10:22|192.168.1.19|社区通知：本周五开展消防演练，请居民配合物业安排。\n"
			"2026-05-14 10:11:36|192.168.1.20|马上删除！别吃这种水果，会立刻导致中毒，已经有人进ICU！\n"
			"2026-05-14 10:12:58|192.168.1.21|校学生会通知：周六晚礼堂举办毕业晚会，欢迎同学报名志愿者。\n"
			"2026-05-14 10:13:41|192.168.1.22|揭秘内部名单：这几类人明天起将被限制出行，赶紧转发！\n"
			"2026-05-14 10:14:09|192.168.1.23|交通部门公告：受施工影响，2路公交临时改道三天。\n"
			"2026-05-14 10:15:30|192.168.1.24|不看后悔！国家刚刚秘密宣布重大政策，普通人必须马上知道！\n"
			"2026-05-14 10:16:45|192.168.1.25|教育部通知：今年高考时间确定为6月7-8日，请考生做好准备。\n"
			"2026-05-14 10:17:22|192.168.1.26|震惊！专家透露吃这种蔬菜会致癌，电视台都不敢播！\n"
			"2026-05-14 10:18:33|192.168.1.27|学校食堂公告：本周菜单已更新，请同学们关注公众号查看。\n"
			"2026-05-14 10:19:51|192.168.1.28|赶紧告诉家人！自来水致癌物质超标，所有人都在转发！\n"
			"2026-05-14 10:20:15|192.168.1.29|市公安局提醒：近期电信诈骗高发，请勿轻信陌生来电。\n"
			"2026-05-14 10:21:38|192.168.1.30|绝对内部消息：某大型企业即将宣布破产，速告知亲友！\n"
			"2026-05-14 10:22:49|192.168.1.31|图书馆自习室开放时间通知：周一至周五 8:00-22:00。\n"
			"2026-05-14 10:23:57|192.168.1.32|出大事了！明天起全国油价暴涨，抓紧去加油！\n"
			"2026-05-14 10:24:21|192.168.1.33|校医院通知：请需要体检的同学于本周五前预约。\n"
			"2026-05-14 10:25:44|192.168.1.34|深度揭秘：某某明星的惊天丑闻，看完睡不着觉！\n";

		writeStringToFile(NEWS_FILE, sampleNews);
	}
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

	const std::string TRACKER_LOG_FILE = DATA_DIR + "tracker_log.txt";
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

	// 加载历史追踪数据（支持跨次运行）
	tracker.loadFromFile(TRACKER_LOG_FILE);

	// 初始化示例数据（如果不存在）
	ensureSampleData();

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
					tracker.recordContent(fp, pkt.content);  // 记录正文供显示用

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
				
				// 只在假新闻时显示详细结果，减少刷屏
				if (isFake) {
					ui.showDetectionResult(pkt.content, dr.credibility, isFake);
				}
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

			// 保存追踪数据（持久化）
			tracker.saveToFile(TRACKER_LOG_FILE);

			ui.showSuccess("监控完成。已自动保存追踪数据。");
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
			ui.setColor(COLOR_CYAN);
			std::cout << R"(
╔══════════════════════════════════════════════════════════╗
║                  假 新 闻 检 索 引 擎                   ║
╠══════════════════════════════════════════════════════════╣
║  支持中英文关键词，短语搜索用空格分隔                     ║
╚══════════════════════════════════════════════════════════╝
)";
			ui.resetColor();
			std::cout << "\n";
			ui.setColor(COLOR_WHITE);
			std::cout << "  请输入关键词：";
			ui.resetColor();
			std::string keyword = ui.getUserInput("");
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
			ui.setColor(COLOR_CYAN);
			std::cout << R"(
╔══════════════════════════════════════════════════════════╗
║              假 新 闻 压 缩 系 统 - 统计报告            ║
╠══════════════════════════════════════════════════════════╣
╚══════════════════════════════════════════════════════════╝
)";
			ui.resetColor();
			ui.setColor(COLOR_BLUE);
			std::cout << "  数据时间: 2026年5月  |  数据来源: 本地监控\n";
			ui.resetColor();
			ui.setColor(COLOR_CYAN);
			std::cout << "╠══════════════════════════════════════════════════════════╣\n";
			ui.resetColor();

			std::cout << "\n";

			std::cout << "【压缩总览】\n";
			ui.setColor(COLOR_WHITE);
			std::cout << "  总新闻数: ";
			ui.setColor(COLOR_CYAN);
			std::cout << lastTotalNews << "\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  假新闻数: ";
			ui.setColor(COLOR_RED);
			std::cout << lastFakeCount << "\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  假新闻占比: ";
			if (lastFakeRatio > 50) ui.setColor(COLOR_RED);
			else ui.setColor(COLOR_GREEN);
			std::cout << lastFakeRatio << "%\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  样本平均压缩率: ";
			if (lastAvgRatio > 100) ui.setColor(COLOR_YELLOW);
			else ui.setColor(COLOR_GREEN);
			std::cout << lastAvgRatio << "%\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  全局字节压缩率: ";
			if (lastGlobalRatio > 100) ui.setColor(COLOR_YELLOW);
			else ui.setColor(COLOR_GREEN);
			std::cout << lastGlobalRatio << "%";
			if (lastGlobalRatio > 100) std::cout << " (数据膨胀)";
			std::cout << "\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  空间节省率: ";
			if (lastSavingRate >= 0) ui.setColor(COLOR_GREEN);
			else ui.setColor(COLOR_RED);
			std::cout << lastSavingRate << "%\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  原始总字节: ";
			ui.setColor(COLOR_CYAN);
			std::cout << lastTotalOriginalBytes << "\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  压缩后字节: ";
			ui.setColor(COLOR_CYAN);
			std::cout << lastTotalCompressedBytes << "\n";
			ui.resetColor();

			ui.setColor(COLOR_WHITE);
			std::cout << "  数据膨胀条数: ";
			ui.setColor(COLOR_YELLOW);
			std::cout << lastExpansionCases << " / " << lastFakeCount << "\n";
			ui.resetColor();

			ui.setColor(COLOR_CYAN);
			std::cout << "\n【传播热点 Top5】（按传播次数排序）\n";
			ui.resetColor();
			std::vector<std::pair<size_t, int>> top5 = tracker.getTopFakeNews(5);
			if (top5.empty()) {
				ui.setColor(COLOR_YELLOW);
				std::cout << "  (暂无数据)\n";
				ui.resetColor();
			}
			else {
				for (size_t i = 0; i < top5.size(); ++i) {
					size_t fp = top5[i].first;
					int count = top5[i].second;
					std::string content = tracker.getContent(fp);
					// 截断长内容避免换行
					if (content.length() > 40) {
						content = content.substr(0, 37) + "...";
					}

					ui.setColor(COLOR_RED);
					std::cout << " " << (i + 1) << ". ";
					ui.resetColor();
					ui.setColor(COLOR_WHITE);
					std::cout << "[" << count << "次传播] ";
					ui.resetColor();
					ui.setColor(COLOR_YELLOW);
					std::cout << content << "\n";
					ui.resetColor();
				}
			}
			std::cout << "=================================================\n";

			ui.waitForKey();
		}
		else if (choice == 4) {
			// ==============================
			// 关于页面
			// ==============================
			ui.clearScreen();
			ui.setColor(COLOR_CYAN);
			std::cout << R"(
+========================================================+
|                    关 于 我 们                         |
+========================================================+

  [项目名称]  假新闻监控与智能压缩系统
  [版本号]    1.0
  [开发者]    QZR1022
  [参赛比赛]  第21届"鼎甲杯"黑框框程序设计大赛
  [课程归属]  大一软件工程专业期末作业

+--------------------------------------------------------+

  [项目简介]
  本系统是一个纯 C++ 实现的假新闻检测与选择性压缩归档
  工具。只有被判定为假新闻的文本才会被压缩存储，同时追
  踪其传播路径，支持关键词检索。

+--------------------------------------------------------+

  [技术亮点]
  * 零第三方依赖 - 纯 C++17 + STL + Windows API
  * 手写 LZ77 压缩算法 - 非调用 zlib 等库
  * 朴素贝叶斯分类器 - 从零实现机器学习模型
  * 传播链追踪 - 内容指纹 + IP 溯源
  * 倒排索引检索 - 支持中英文混合搜索
  * 彩色控制台界面 - 专业级 TUI 设计

+--------------------------------------------------------+

  [数据格式说明]
  news.txt 文件格式（每行一条）：
  时间戳|来源IP|新闻正文

  示例：
  2026-05-14 10:00:01|192.168.1.1|震惊！林丹决赛前药检阳性！

+--------------------------------------------------------+

  [系统架构]
  main.cpp -> ConsoleUI（界面）
           -> Detector（假新闻检测）
           -> LZ77（压缩算法）
           -> Tracker（传播追踪）
           -> Searcher（关键词检索）
           -> PacketReader（数据读取）
           -> Utils（工具函数）

+--------------------------------------------------------+

  [项目目录]
  FakeNewsCompressor/
  ├── main.cpp           主入口
  ├── ConsoleUI.cpp/h    控制台界面
  ├── Detector.cpp/h     假新闻检测
  ├── LZ77.cpp/h         LZ77压缩算法
  ├── Tracker.cpp/h       传播追踪
  ├── Searcher.cpp/h      关键词检索
  ├── PacketReader.cpp/h  数据读取
  ├── Weights.h           模型权重
  ├── Config.h            全局配置
  ├── Utils.cpp/h         工具函数
  └── data/
      ├── news.txt        输入数据
      ├── archive/        压缩文件(.ncz)
      └── tracker_log.txt 追踪记录

+========================================================+
)";
			ui.resetColor();
			ui.waitForKey();
		}
		else if (choice == 5) {
			running = false;
			ui.showInfo("Program exited. Goodbye!");
		}
	}

	return 0;
}