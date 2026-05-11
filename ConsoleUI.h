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
    //       fakeCount - 发现假新闻数量，avgRatio - 平均压缩率
    void showMonitorPanel(int current, int total, int fakeCount, double avgRatio);

    // 显示进度条
    // 参数：current - 当前进度，total - 总值，width - 进度条宽度
    void showProgressBar(int current, int total, int width);

    // 显示假新闻检测结果
    // 参数：content - 新闻内容，credibility - 可信度，isFake - 是否假新闻
    void showDetectionResult(const std::string& content, double credibility, bool isFake);

    // 显示检索结果列表
    // 参数：results - 搜索结果列表
    void showSearchResults(const std::vector<class SearchResult>& results);

    // 显示统计报告
    // 参数：totalNews - 总新闻数，fakeCount - 假新闻数
    //       avgCompressionRatio - 平均压缩率，topFakeList - 热点假新闻列表
    void showStatistics(int totalNews, int fakeCount, double avgCompressionRatio,
        const std::vector<std::pair<size_t, int>>& topFakeList);

    // 显示信息（普通提示）
    void showInfo(const std::string& message);

    // 显示成功信息（绿色）
    void showSuccess(const std::string& message);

    // 显示警告信息（黄色）
    void showWarning(const std::string& message);

    // 显示错误信息（红色）
    void showError(const std::string& message);

    // 清空屏幕
    void clearScreen();

    // 等待用户按任意键继续
    void waitForKey();

    // 获取用户输入字符串
    std::string getUserInput(const std::string& prompt);

    // 获取用户输入整数（带范围验证）
    int getUserChoice(int min, int max);

private:
    // 设置文本颜色
    void setColor(int colorCode);

    // 恢复默认颜色
    void resetColor();

    // 保存原始控制台设置（用于析构时恢复）
    HANDLE m_consoleHandle;
    CONSOLE_SCREEN_BUFFER_INFO m_originalConsoleInfo;
};

#endif // CONSOLEUI_H

