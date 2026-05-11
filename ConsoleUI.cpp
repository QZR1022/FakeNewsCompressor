#include "ConsoleUI.h"
#include "Config.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <conio.h>

ConsoleUI::ConsoleUI() {
	m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(m_consoleHandle, &m_originalConsoleInfo);
}

ConsoleUI::~ConsoleUI() {
	// 恢复原颜色
	SetConsoleTextAttribute(m_consoleHandle, m_originalConsoleInfo.wAttributes);
}

void ConsoleUI::setColor(int colorCode) {
	SetConsoleTextAttribute(m_consoleHandle, (WORD)colorCode);
}

void ConsoleUI::resetColor() {
	SetConsoleTextAttribute(m_consoleHandle, (WORD)COLOR_DEFAULT);
}

void ConsoleUI::clearScreen() {
	system("cls");
}

void ConsoleUI::showSplashScreen() {
	clearScreen();
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
	setColor(COLOR_CYAN);
	std::cout << "============ MAIN MENU ============\n";
	resetColor();

	std::cout << "1. Start Monitoring\n";
	std::cout << "2. Search News\n";
	std::cout << "3. Show Statistics\n";
	std::cout << "4. Exit\n";
	std::cout << "===================================\n";
	std::cout << "Please enter choice (1-4): ";

	return getUserChoice(1, 4);
}

void ConsoleUI::showProgressBar(int current, int total, int width) {
	if (total <= 0) total = 1;
	if (current < 0) current = 0;
	if (current > total) current = total;
	if (width <= 0) width = PROGRESS_BAR_WIDTH;

	double ratio = (double)current / (double)total;
	int filled = (int)(ratio * width);

	std::cout << "[";
	for (int i = 0; i < width; ++i) {
		if (i < filled) std::cout << "#";
		else std::cout << "-";
	}
	std::cout << "] " << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
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

	setColor(COLOR_GREEN);
	std::cout << "Avg Compression Ratio: " << std::fixed << std::setprecision(2) << avgRatio << "%\n";
	resetColor();

	std::cout << "=======================================\n";
}

void ConsoleUI::showDetectionResult(const std::string& content, double credibility, bool isFake) {
	setColor(COLOR_CYAN);
	std::cout << "\n--- Detection Result ---\n";
	resetColor();

	std::cout << "Content: " << content << "\n";
	std::cout << "Credibility: " << std::fixed << std::setprecision(2) << credibility << "\n";

	if (isFake) {
		setColor(COLOR_RED);
		std::cout << "Label: FAKE NEWS\n";
	}
	else {
		setColor(COLOR_GREEN);
		std::cout << "Label: REAL/SAFE\n";
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
			<< " | Time=" << r.timestamp << "\n";
		std::cout << "Content: " << r.content << "\n";
		std::cout << "----------------------------------------\n";
	}
}

void ConsoleUI::showStatistics(int totalNews, int fakeCount, double avgCompressionRatio,
	double fakeRatio,
	const std::vector<std::pair<size_t, int>>& topFakeList) {
	clearScreen();

	setColor(COLOR_CYAN);
	std::cout << "=============== STATISTICS ===============\n";
	resetColor();

	std::cout << "Total News: " << totalNews << "\n";

	setColor(COLOR_RED);
	std::cout << "Fake News: " << fakeCount << "\n";
	resetColor();

	setColor(COLOR_YELLOW);
	std::cout << "Fake Ratio: " << std::fixed << std::setprecision(2)
		<< fakeRatio << "%\n";
	resetColor();

	setColor(COLOR_GREEN);
	std::cout << "Average Compression Ratio: " << std::fixed << std::setprecision(2)
		<< avgCompressionRatio << "%\n";
	resetColor();

	std::cout << "\nTop Fake News (by propagation):\n";
	if (topFakeList.empty()) {
		std::cout << " (empty)\n";
	}
	else {
		for (size_t i = 0; i < topFakeList.size(); ++i) {
			std::cout << " " << (i + 1) << ". Fingerprint=" << topFakeList[i].first
				<< ", Count=" << topFakeList[i].second << "\n";
		}
	}

	std::cout << "==========================================\n";
}

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