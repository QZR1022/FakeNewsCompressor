#ifndef CONSOLEUI_H
#define CONSOLEUI_H

// ============================================
// ConsoleUI.h - 控制台界面类头文件
// ============================================
// 功能：提供美观的彩色 TUI 界面，包括主菜单、监控面板、
//       搜索结果、统计报告等显示功能
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>
#include <vector>
#include <windows.h>
#include "Searcher.h"

// 控制台界面类
// 职责：封装所有控制台输出逻辑，包括颜色控制、界面绘制、用户交互
class ConsoleUI {
public:
	// ============================================
	// 构造函数
	// 功能：获取控制台句柄，保存原始控制台属性
	// 说明：程序结束时通过析构函数恢复默认颜色
	// ============================================
	ConsoleUI();

	// ============================================
	// 析构函数
	// 功能：恢复程序启动时的控制台颜色设置
	// ============================================
	~ConsoleUI();

	// ============================================
	// showSplashScreen: 显示启动画面（ASCII Art）
	// 功能：程序启动时显示项目 logo 和欢迎信息
	// 说明：使用预格式化的 ASCII 艺术字展示项目名称
	// ============================================
	void showSplashScreen();

	// ============================================
	// init: 初始化界面
	// 功能：显示启动画面并等待用户按键继续
	// 说明：通常在程序入口处调用
	// ============================================
	void init();

	// ============================================
	// showMainMenu: 显示主菜单
	// 功能：绘制主菜单界面，获取用户输入
	// 返回：用户选择的菜单项（1-5）
	// 说明：菜单选项包括监控、检索、统计、关于、退出
	// ============================================
	int showMainMenu();

	// ============================================
	// showMonitorPanel: 显示监控面板（动态刷新）
	// 参数说明：
	//   - current: 当前处理第几条
	//   - total: 总条数
	//   - fakeCount: 发现假新闻数量
	//   - avgRatio: 平均压缩率（百分比）
	// 功能：实时显示监控进度和关键指标
	// 说明：在监控循环中每处理一条新闻调用一次
	// ============================================
	void showMonitorPanel(int current, int total, int fakeCount, double avgRatio);

	// ============================================
	// showProgressBar: 显示进度条
	// 参数说明：
	//   - current: 当前进度
	//   - total: 总量
	//   - width: 进度条字符宽度
	// 功能：将进度转换为可视化进度条
	// ============================================
	void showProgressBar(int current, int total, int width);

	// ============================================
	// showDetectionResult: 显示单条新闻检测结果
	// 参数说明：
	//   - content: 新闻正文
	//   - credibility: 可信度（0-100）
	//   - isFake: 是否为假新闻
	// 功能：显示检测结果的详细信息
	// 说明：仅在发现假新闻时调用以减少刷屏
	// ============================================
	void showDetectionResult(const std::string& content, double credibility, bool isFake);

	// ============================================
	// showSearchResults: 显示检索结果列表
	// 参数说明：
	//   - results: SearchResult 结构体向量
	// 功能：以表格形式展示搜索结果
	// 说明：每条结果包含编号、可信度、匹配次数、时间、内容
	// ============================================
	void showSearchResults(const std::vector<class SearchResult>& results);

	// ============================================
	// showStatistics: 显示统计报告
	// 参数说明：
	//   - totalNews: 总新闻数
	//   - fakeNews: 假新闻数
	//   - avgCompressionRatio: 平均压缩率
	//   - fakeRatio: 假新闻占比
	//   - topList: 热点假新闻列表（指纹+次数）
	// 功能：展示监控统计汇总信息
	// ============================================
	void showStatistics(int totalNews, int fakeNews, double avgCompressionRatio,
		double fakeRatio,
		const std::vector<std::pair<size_t, int>>& topList);

	// ============================================
	// 消息输出函数（带颜色标记）
	// ============================================
	void showInfo(const std::string& message);      // 信息（白色）
	void showSuccess(const std::string& message);  // 成功（绿色）
	void showWarning(const std::string& message);  // 警告（黄色）
	void showError(const std::string& message);    // 错误（红色）

	// ============================================
	// 控制函数
	// ============================================
	void clearScreen();   // 清屏（使用 system("cls")）
	void waitForKey();    // 等待用户按键

	// ============================================
	// 用户输入函数
	// ============================================
	std::string getUserInput(const std::string& prompt);  // 获取一行文本输入
	int getUserChoice(int min, int max);                  // 获取数字菜单选择

	// ============================================
	// 颜色控制函数（公开给 main.cpp 使用）
	// ============================================
	void setColor(int colorCode);   // 设置控制台前景色
	void resetColor();             // 恢复默认颜色

private:
	// ============================================
	// 私有成员
	// ============================================
	HANDLE m_consoleHandle;              // Windows 控制台句柄
	CONSOLE_SCREEN_BUFFER_INFO m_originalConsoleInfo;  // 保存原始控制台信息
};

#endif // CONSOLEUI_H