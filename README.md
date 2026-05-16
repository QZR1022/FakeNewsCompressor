# FakeNewsCompressor

> 假新闻监控与智能压缩系统 | Fake News Monitor & Intelligent Compression System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-green.svg)](https://www.microsoft.com/windows)
[![License](https://img.shields.io/badge/License-MIT-orange.svg)](LICENSE)
[![Contest](https://img.shields.io/badge/Contest-鼎甲杯第21届-red.svg)](https://github.com/QZR1022/FakeNewsCompressor)

---

## 一句话介绍 | One-Line Description

一个纯 C++ 实现的假新闻检测与选择性压缩归档工具，只有被判定为假新闻的文本才会被压缩存储，同时追踪其传播路径，支持关键词检索。

A pure C++ implementation of fake news detection and selective compression archive tool.

---

## 功能特性 | Features

| 功能 | 描述 |
|------|------|
| **假新闻检测** | 基于朴素贝叶斯分类器，5 维特征提取（感叹号、问号、大写比例、敏感词、句长） |
| **智能压缩** | 自主实现 LZ77 压缩算法，仅对假新闻进行压缩存储 |
| **传播追踪** | 通过内容指纹追踪假新闻的传播路径和来源 IP |
| **关键词检索** | 倒排索引 + 全文 fallback，支持中英文混合搜索 |
| **美观的界面** | Windows 控制台 TUI，彩色输出，动态进度条 |

---

## 快速开始 | Quick Start

### 编译 | Build

```bash
# 使用 Visual Studio 2022 打开项目
# 选择 Release / Debug 配置
# 按 Ctrl + Shift + B 编译
# 可执行文件位于 x64/Release/ 或 x64/Debug/
```

### 运行 | Run

```bash
# 直接运行（程序会自动创建示例数据）
FakeNewsCompressor.exe
```

### 数据格式 | Data Format

```text
news.txt 每行格式：时间戳|IP地址|新闻内容

示例：
2026-05-14 10:00:01|192.168.1.1|震惊！林丹决赛前药检阳性！
2026-05-14 10:00:02|192.168.1.2|中国队夺得金牌，恭喜！
```

---

## 界面预览 | UI Preview

```
    ____  ____  ____  ____  ____  ____  _________
   / / / / __ \/ __ \/ __ \/ __ \/ __ \/ ___/ __ \
  / / / / /_/ / /_/ / / / / / / / / / / __/ /_/ /
 / / / / __/ / __// /_/ / /_/ / /_/ / __/ / __/
/ / / / /_/ / /_  / __/ / __// __/ / /_/ / /_ 
\/_/\____/\____/_/   /_/  /_/ /_/ /_/\____/\____/

+========================================================+
|          FAKE NEWS COMPRESSOR - MAIN MENU             |
+========================================================+

  [1] START MONITORING
      Read news, detect and compress fake news

  [2] SEARCH ARCHIVE
      Search archived fake news by keywords

  [3] VIEW STATISTICS
      Compression rate and top fake news ranking

  [4] ABOUT
      Project info and technical highlights

  [5] EXIT
```

---

## 技术架构 | Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    FakeNewsCompressor                   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   ┌─────────────┐                                      │
│   │   main.cpp  │ ← 主入口，菜单循环                    │
│   └──────┬──────┘                                      │
│          │                                              │
│   ┌──────▼──────┐                                      │
│   │ ConsoleUI   │ ← 彩色 TUI，进度条，菜单              │
│   └──────┬──────┘                                      │
│          │                                              │
│   ┌──────▼──────┐      ┌─────────────┐                │
│   │ PacketReader│─────▶│  data/      │                │
│   └──────┬──────┘      └─────────────┘                │
│          │                                              │
│   ┌──────▼──────┐      ┌─────────────┐                │
│   │  Detector   │─────▶│  Weights.h  │ ← 模型权重      │
│   └──────┬──────┘      └─────────────┘                │
│          │                                              │
│          │ 如果是假新闻                                  │
│   ┌──────▼──────┐      ┌─────────────┐                │
│   │    LZ77     │─────▶│  *.ncz      │ ← 压缩文件      │
│   └──────┬──────┘      └─────────────┘                │
│          │                                              │
│   ┌──────▼──────┐      ┌─────────────┐                │
│   │   Tracker   │─────▶│ tracker_log │ ← 追踪记录      │
│   └─────────────┘      └─────────────┘                │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## 技术亮点 | Technical Highlights

- **零第三方依赖** - 纯 C++17 + STL + Windows API，无任何外部库
- **手写压缩算法** - 自主实现 LZ77 滑动窗口压缩，非调用 zlib
- **机器学习落地** - 从零实现朴素贝叶斯分类器，权重可配置
- **模块化设计** - 高内聚低耦合，便于扩展和维护
- **跨次运行** - 自动持久化追踪数据，跨会话累计统计

---

## 项目结构 | Project Structure

```
FakeNewsCompressor/
├── main.cpp           # 主入口，菜单逻辑
├── ConsoleUI.cpp/h    # 控制台界面
├── Detector.cpp/h     # 假新闻检测
├── LZ77.cpp/h         # LZ77 压缩算法
├── Tracker.cpp/h      # 传播追踪
├── Searcher.cpp/h     # 关键词检索
├── PacketReader.cpp/h # 数据读取
├── Weights.h          # 模型权重
├── Config.h           # 全局配置
├── Utils.cpp/h        # 工具函数
└── data/
    ├── news.txt       # 输入数据
    ├── archive/       # 压缩输出 (.ncz)
    └── tracker_log.txt # 追踪记录
```

---

## 参赛信息 | Contest Info

- **比赛**: 第21届"鼎甲杯"黑框框程序设计大赛
- **课程**: 大一软件工程专业期末作业
- **开发周期**: 20天 (2026-05-04 ~ 2026-05-24)
- **代码规模**: 约 2500 行 C++

---

## 开发者 | Developer

- **Author**: QZR1022
- **GitHub**: https://github.com/QZR1022/FakeNewsCompressor
- **Version**: 1.0
- **Last Update**: 2026-05-16

---

## 许可证 | License

MIT License - 欢迎 Star 和 Fork！

*Made with C++ and passion for fighting fake news.*