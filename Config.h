#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Config.h - 全局配置头文件
// ============================================
// 功能：定义系统全局配置常量
// 包括：文件路径、压缩参数、系统设置、颜色配置
// 作者：QZR1022
// 版本：1.0
// ============================================

#include <string>

// ============================================
// 文件路径配置
// 说明：所有文件路径相对于程序运行目录
// ============================================
const std::string DATA_DIR = "./data/";           // 数据目录（相对路径）
const std::string NEWS_FILE = DATA_DIR + "news.txt";        // 新闻输入文件
const std::string ARCHIVE_DIR = DATA_DIR + "archive/";      // 压缩归档目录
const std::string COMPRESSED_EXT = ".ncz";        // 压缩文件扩展名

// ============================================
// LZ77 压缩配置
// 说明：滑动窗口算法参数
// ============================================
const int LZ77_WINDOW_SIZE = 4096;      // 滑动窗口大小（4KB）
                                              // 作用：决定搜索历史数据的范围
                                              // 影响：窗口越大，压缩效果越好，但内存占用越高
const int LZ77_LOOKAHEAD_SIZE = 16;     // 前瞻缓冲区大小（16字节）
                                              // 作用：限制最长匹配的长度
                                              // 影响：前瞻缓冲区越大，最长匹配越长

// ============================================
// 系统配置
// ============================================
const int MAX_NEWS_COUNT = 1000;        // 最大处理新闻数（预留）
const int PROGRESS_BAR_WIDTH = 40;       // 进度条宽度（字符数）

// ============================================
// 颜色配置（Windows 控制台）
// 说明：使用 Windows API SetConsoleTextAttribute 设置颜色
// 使用方式：setColor(COLOR_RED); ... resetColor();
// ============================================
const int COLOR_DEFAULT = 7;            // 白色（默认/普通文本）
const int COLOR_WHITE = 7;              // 白色（普通文本）
const int COLOR_GREEN = 10;            // 绿色（成功/正常/真新闻）
const int COLOR_RED = 12;              // 红色（警告/假新闻/错误）
const int COLOR_YELLOW = 14;           // 黄色（注意/标题/高亮）
const int COLOR_CYAN = 11;             // 青色（主标题/边框）
const int COLOR_BLUE = 9;              // 蓝色（统计/信息）

#endif // CONFIG_H