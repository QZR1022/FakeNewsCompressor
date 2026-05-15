#include "ConsoleUI.h"
#include "Config.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <conio.h>
#include <cstdlib> // system

// =========================
// 构造 / 析构
// =========================
ConsoleUI::ConsoleUI() {
	// 获取标准输出句柄
	m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	// 保存程序启动时的原始控制台属性（颜色等）
	GetConsoleScreenBufferInfo(m_consoleHandle, &m_originalConsoleInfo);
}

ConsoleUI::~ConsoleUI() {
	// 程序结束时恢复原始控制台属性
	SetConsoleTextAttribute(m_consoleHandle, m_originalConsoleInfo.wAttributes);
}

// =========================
// 颜色与基础控制
// =========================
void ConsoleUI::setColor(int colorCode) {
	// 设置当前输出颜色
	// colorCode 通常来自 Config.h，例如 COLOR_RED/COLOR_GREEN/COLOR_CYAN
	SetConsoleTextAttribute(m_consoleHandle, static_cast<WORD>(colorCode));
}

void ConsoleUI::resetColor() {
	// 恢复为程序启动时的默认颜色
	SetConsoleTextAttribute(m_consoleHandle, m_originalConsoleInfo.wAttributes);
}

void ConsoleUI::clearScreen() {
	// 清空控制台
	system("cls");
}

// =========================
// 页面显示
// =========================
void ConsoleUI::showSplashScreen() {
	clearScreen();

	// 标题用青色
	setColor(COLOR_CYAN);
	std::cout << "=============================================\n";
	std::cout << " NetCompress System \n";
	std::cout << " Fake News Monitor & Intelligent Compress \n";
	std::cout << "=============================================\n";
	resetColor();

	std::cout << "Version: 1.0\n";
	std::cout << "No third-party libs | C++ + Windows Console\n\n";
}

int ConsoleUI::showMainMenu() {
	clearScreen();

	// 标题：青色
	setColor(COLOR_CYAN);
	std::cout << "============ MAIN MENU ============\n";
	resetColor();

	// 1 监控：绿色
	setColor(COLOR_GREEN);
	std::cout << "1. Start Monitoring\n";
	resetColor();

	// 2 检索：黄色
	setColor(COLOR_YELLOW);
	std::cout << "2. Search News\n";
	resetColor();

	// 3 统计：蓝色（如果你Config里没有COLOR_BLUE，就先用COLOR_CYAN）
	setColor(COLOR_BLUE);
	std::cout << "3. Show Statistics\n";
	resetColor();

	// 4 退出：红色
	setColor(COLOR_RED);
	std::cout << "4. Exit\n";
	resetColor();

	std::cout << "===================================\n";
	std::cout << "Please enter choice (1-4): ";

	return getUserChoice(1, 4);
}

void ConsoleUI::showProgressBar(int current, int total, int width) {
	if (total <= 0) total = 1;
	if (current < 0) current = 0;
	if (current > total) current = total;
	if (width <= 0) width = PROGRESS_BAR_WIDTH;

	double ratio = static_cast<double>(current) / static_cast<double>(total);
	int filled = static_cast<int>(ratio * width);

	std::cout << "[";
	for (int i = 0; i < width; ++i) {
		std::cout << (i < filled ? "#" : "-");
	}
	std::cout << "] "
		<< std::fixed << std::setprecision(1)
		<< (ratio * 100.0) << "%";
}

void ConsoleUI::showMonitorPanel(int current, int total, int fakeCount, double avgRatio) {
	clearScreen();

	// 面板标题：青色
	setColor(COLOR_CYAN);
	std::cout << "============ MONITOR PANEL ============\n";
	resetColor();

	std::cout << "Progress: ";
	showProgressBar(current, total, PROGRESS_BAR_WIDTH);
	std::cout << "\n";

	std::cout << "Processed: " << current << " / " << total << "\n";

	// 假新闻计数：红色
	setColor(COLOR_RED);
	std::cout << "Fake News Found: " << fakeCount << "\n";
	resetColor();

	// 压缩率：
	// <=100% 用绿色（压缩有效）
	// >100% 用黄色（膨胀提醒）
	if (avgRatio <= 100.0) setColor(COLOR_GREEN);
	else setColor(COLOR_YELLOW);

	std::cout << "Avg Compression Ratio: "
		<< std::fixed << std::setprecision(2)
		<< avgRatio << "%";
	if (avgRatio > 100.0) {
		std::cout << " (Expanded)";
	}
	std::cout << "\n";
	resetColor();

	std::cout << "=======================================\n";
}

void ConsoleUI::showDetectionResult(const std::string& content, double credibility, bool isFake) {
	// 小标题：青色
	setColor(COLOR_CYAN);
	std::cout << "\n--- Detection Result ---\n";
	resetColor();

	std::cout << "Content: " << content << "\n";
	std::cout << "Credibility: "
		<< std::fixed << std::setprecision(2)
		<< credibility << "\n";

	// 标签颜色：假新闻红色，正常绿色
	if (isFake) {
		setColor(COLOR_RED);
		std::cout << "Label: FAKE NEWS\n";
	}
	else {
		setColor(COLOR_GREEN);
		std::cout << "Label: REAL / SAFE\n";
	}
	resetColor();

	std::cout << "------------------------\n";
}

void ConsoleUI::showSearchResults(const std::vector<SearchResult>& results) {
	// 标题：青色
	setColor(COLOR_CYAN);
	std::cout << "\n============ SEARCH RESULTS ============\n";
	resetColor();

	if (results.empty()) {
		// 无结果：黄色提醒
		setColor(COLOR_YELLOW);
		std::cout << "No matched results.\n";
		resetColor();
		return;
	}

	for (size_t i = 0; i < results.size(); ++i) {
		const SearchResult& r = results[i];

		std::cout << "[" << i + 1 << "] "
			<< "ID=" << r.newsId
			<< " | Match=" << r.matchCount
			<< " | Cred=" << std::fixed << std::setprecision(2) << r.credibility
			<< " | Mode=" << (r.usedFallback ? "FULLTEXT" : "INDEX")
			<< " | Time=" << r.timestamp << "\n";

		std::cout << "Content: " << r.content << "\n";
		std::cout << "----------------------------------------\n";
	}
}

void ConsoleUI::showStatistics(int totalNews, int fakeNews, double avgCompressionRatio,
	double fakeRatio,
	const std::vector<std::pair<size_t, int>>& topList) {
	clearScreen();

	// 标题：青色
	setColor(COLOR_CYAN);
	std::cout << "=============== STATISTICS ===============\n";
	resetColor();

	std::cout << "Total News: " << totalNews << "\n";

	// 假新闻数量：红色
	setColor(COLOR_RED);
	std::cout << "Fake News: " << fakeNews << "\n";
	resetColor();

	// 假新闻占比：黄色
	setColor(COLOR_YELLOW);
	std::cout << "Fake Ratio: "
		<< std::fixed << std::setprecision(2)
		<< fakeRatio << "%\n";
	resetColor();

	// 压缩率：
	// <=100% 绿色
	// >100% 黄色并标记 Expanded
	if (avgCompressionRatio <= 100.0) setColor(COLOR_GREEN);
	else setColor(COLOR_YELLOW);

	std::cout << "Compression Ratio (compressed/original): "
		<< std::fixed << std::setprecision(2)
		<< avgCompressionRatio << "%";
	if (avgCompressionRatio > 100.0) {
		std::cout << " [Expanded]";
	}
	std::cout << "\n";
	resetColor();

	// 节省率：>=0绿色，<0红色
	double savingRate = 100.0 - avgCompressionRatio;
	if (savingRate >= 0.0) setColor(COLOR_GREEN);
	else setColor(COLOR_RED);

	std::cout << "Space Saving Rate: "
		<< std::fixed << std::setprecision(2)
		<< savingRate << "%\n";
	resetColor();

	std::cout << "\nTop Fake News (by propagation):\n";
	if (topList.empty()) {
		std::cout << " (empty)\n";
	}
	else {
		for (size_t i = 0; i < topList.size(); ++i) {
			std::cout << " " << (i + 1)
				<< ". Fingerprint=" << topList[i].first
				<< ", Count=" << topList[i].second << "\n";
		}
	}

	std::cout << "==========================================\n";
}

// =========================
// 消息输出
// =========================
void ConsoleUI::showInfo(const std::string& message) {
	resetColor();
	std::cout << "[INFO] " << message << "\n";
}

void ConsoleUI::showSuccess(const std::string& message) {
	// 成功：绿色
	setColor(COLOR_GREEN);
	std::cout << "[SUCCESS] " << message << "\n";
	resetColor();
}

void ConsoleUI::showWarning(const std::string& message) {
	// 警告：黄色
	setColor(COLOR_YELLOW);
	std::cout << "[WARNING] " << message << "\n";
	resetColor();
}

void ConsoleUI::showError(const std::string& message) {
	// 错误：红色
	setColor(COLOR_RED);
	std::cout << "[ERROR] " << message << "\n";
	resetColor();
}

// =========================
// 输入相关
// =========================
void ConsoleUI::waitForKey() {
	std::cout << "\nPress any key to continue...";
	_getch();
	std::cout << "\n";
}

std::string ConsoleUI::getUserInput(const std::string& prompt) {
	std::cout << prompt;
	std::string input;
	std::getline(std::cin, input);
	return input;
}

int ConsoleUI::getUserChoice(int min, int max) {
	int choice;

	while (true) {
		std::cin >> choice;

		// 非数字输入
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
			std::cout << "Invalid input, please enter number (" << min << "-" << max << "): ";
			continue;
		}

		// 清理输入缓冲
		std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

		// 范围校验
		if (choice < min || choice > max) {
			std::cout << "Out of range, please enter (" << min << "-" << max << "): ";
			continue;
		}

		return choice;
	}
}