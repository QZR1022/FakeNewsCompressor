#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <string>
#include <vector>
#include <windows.h>
#include "Searcher.h"

// 控制台界面类
class ConsoleUI {
public:
	// 构造函数：初始化控制台
	ConsoleUI();

	// 析构函数：恢复默认设置
	~ConsoleUI();

	// 显示启动画面（ASCII Art）
	void showSplashScreen();

	// 显示主菜单，返回用户选择
	// 返回：1-监控, 2-检索, 3-统计, 4-退出
	int showMainMenu();

	// 显示监控面板（动态刷新）
	// 参数：current - 当前处理第几条，total - 总条数
	// fakeCount - 发现假新闻数量，avgRatio - 平均压缩率（%）
	void showMonitorPanel(int current, int total, int fakeCount, double avgRatio);

	// 显示进度条
	void showProgressBar(int current, int total, int width);

	// 显示假新闻检测结果
	void showDetectionResult(const std::string& content, double credibility, bool isFake);

	// 显示检索结果列表
	void showSearchResults(const std::vector<class SearchResult>& results);

	// 显示统计报告
	// avgCompressionRatio: 压缩率(%) = compressed/original*100
	void showStatistics(int totalNews, int fakeNews, double avgCompressionRatio,
		double fakeRatio,
		const std::vector<std::pair<size_t, int>>& topList);

	// 消息
	void showInfo(const std::string& message);
	void showSuccess(const std::string& message);
	void showWarning(const std::string& message);
	void showError(const std::string& message);

	// 控制
	void clearScreen();
	void waitForKey();

	// 输入
	std::string getUserInput(const std::string& prompt);
	int getUserChoice(int min, int max);

private:
	void setColor(int colorCode);
	void resetColor();

	HANDLE m_consoleHandle;
	CONSOLE_SCREEN_BUFFER_INFO m_originalConsoleInfo;
};

#endif // CONSOLEUI_H