#include "ConsoleUI.h"
#include "Config.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <conio.h>
#include <cstdlib>

// =========================
// 构造 / 析构
// =========================
ConsoleUI::ConsoleUI() {
	m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(m_consoleHandle, &m_originalConsoleInfo);
}

ConsoleUI::~ConsoleUI() {
	SetConsoleTextAttribute(m_consoleHandle, m_originalConsoleInfo.wAttributes);
}

// =========================
// 颜色与基础控制
// =========================
void ConsoleUI::setColor(int colorCode) {
	SetConsoleTextAttribute(m_consoleHandle, static_cast<WORD>(colorCode));
}

void ConsoleUI::resetColor() {
	SetConsoleTextAttribute(m_consoleHandle, m_originalConsoleInfo.wAttributes);
}

void ConsoleUI::clearScreen() {
	system("cls");
}

// =========================
// 页面显示
// =========================
void ConsoleUI::showSplashScreen() {
	clearScreen();

	setColor(COLOR_CYAN);
	std::cout << R"(
    ____  ____  ____  ____  ____  ____  _________
   / / / / __ \/ __ \/ __ \/ __ \/ __ \/ ___/ __ \
  / / / / /_/ / /_/ / / / / / / / / / / __/ /_/ /
 / / / / __/ / __// /_/ / /_/ / /_/ / __/ / __/
/ / / / /_/ / /_  / __/ / __// __  / /_/ / /_ 
\/_/\____/\____/_/   /_/  /_/ /_/ /_/\____/\____/

  Fake News Monitor & Intelligent Compression System
  Ver 1.0 | C++17 | Zero Dependencies | Contest Entry
)";
	resetColor();

	std::cout << "\n";
	setColor(COLOR_YELLOW);
	std::cout << "  [Tech Stack] C++17 + STL + Windows API\n";
	std::cout << "  [Features]   Detection | Compression | Tracking | Search\n";
	resetColor();
	std::cout << "\n";
}

int ConsoleUI::showMainMenu() {
	clearScreen();

	setColor(COLOR_CYAN);
	std::cout << R"(
+========================================================+
|          FAKE NEWS COMPRESSOR - MAIN MENU             |
+========================================================+)";
	resetColor();

	std::cout << "\n";

	setColor(COLOR_GREEN);
	std::cout << "  [1] START MONITORING\n";
	resetColor();
	std::cout << "      Read news, detect and compress fake news\n";

	setColor(COLOR_YELLOW);
	std::cout << "  [2] SEARCH ARCHIVE\n";
	resetColor();
	std::cout << "      Search archived fake news by keywords\n";

	setColor(COLOR_BLUE);
	std::cout << "  [3] VIEW STATISTICS\n";
	resetColor();
	std::cout << "      Compression rate and top fake news ranking\n";

	setColor(COLOR_CYAN);
	std::cout << "  [4] ABOUT\n";
	resetColor();
	std::cout << "      Project info and technical highlights\n";

	setColor(COLOR_RED);
	std::cout << "  [5] EXIT\n";
	resetColor();

	std::cout << "\n";
	setColor(COLOR_CYAN);
	std::cout << "+========================================================+\n";
	resetColor();
	std::cout << "  Enter choice (1-5): ";

	return getUserChoice(1, 5);
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

	setColor(COLOR_CYAN);
	std::cout << "============ MONITOR PANEL ============\n";
	resetColor();

	std::cout << "Progress: ";
	showProgressBar(current, total, PROGRESS_BAR_WIDTH);
	std::cout << "\n";

	std::cout << "Processed: " << current << " / " << total << "\n";

	setColor(COLOR_RED);
	std::cout << "Fake News Found: " << fakeCount << "\n";
	resetColor();

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
	setColor(COLOR_CYAN);
	std::cout << "\n--- Detection Result ---\n";
	resetColor();

	std::cout << "Content: " << content << "\n";
	std::cout << "Credibility: "
		<< std::fixed << std::setprecision(2)
		<< credibility << "\n";

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
	setColor(COLOR_CYAN);
	std::cout << "\n============ SEARCH RESULTS ============\n";
	resetColor();

	if (results.empty()) {
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

	setColor(COLOR_CYAN);
	std::cout << "=============== STATISTICS ===============\n";
	resetColor();

	std::cout << "Total News: " << totalNews << "\n";

	setColor(COLOR_RED);
	std::cout << "Fake News: " << fakeNews << "\n";
	resetColor();

	setColor(COLOR_YELLOW);
	std::cout << "Fake Ratio: "
		<< std::fixed << std::setprecision(2)
		<< fakeRatio << "%\n";
	resetColor();

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
	setColor(COLOR_GREEN);
	std::cout << "[SUCCESS] " << message << "\n";
	resetColor();
}

void ConsoleUI::showWarning(const std::string& message) {
	setColor(COLOR_YELLOW);
	std::cout << "[WARNING] " << message << "\n";
	resetColor();
}

void ConsoleUI::showError(const std::string& message) {
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

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
			std::cout << "Invalid input, please enter number (" << min << "-" << max << "): ";
			continue;
		}

		std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

		if (choice < min || choice > max) {
			std::cout << "Out of range, please enter (" << min << "-" << max << "): ";
			continue;
		}

		return choice;
	}
}