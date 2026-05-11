
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// 文件路径配置
const std::string DATA_DIR = "./data/";
const std::string NEWS_FILE = DATA_DIR + "news.txt";
const std::string ARCHIVE_DIR = DATA_DIR + "archive/";
const std::string COMPRESSED_EXT = ".ncz";

// LZ77 压缩配置
const int LZ77_WINDOW_SIZE = 4096;      // 滑动窗口大小 4KB
const int LZ77_LOOKAHEAD_SIZE = 16;     // 前瞻缓冲区大小 16字节

// 系统配置
const int MAX_NEWS_COUNT = 1000;        // 最大处理新闻数
const int PROGRESS_BAR_WIDTH = 40;      // 进度条宽度（字符数）

// 颜色配置（Windows 控制台）
const int COLOR_DEFAULT = 7;            // 白色
const int COLOR_GREEN = 10;             // 绿色（成功/正常）
const int COLOR_RED = 12;               // 红色（警告/假新闻）
const int COLOR_YELLOW = 14;            // 黄色（注意）
const int COLOR_CYAN = 11;              // 青色（标题）

#endif // CONFIG_H
