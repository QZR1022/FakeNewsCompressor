#ifndef PACKET_READER_H
#define PACKET_READER_H

#include <string>
#include <vector>

// 单条新闻的数据结构
struct NewsPacket {
    std::string timestamp;   // 时间戳，格式 "YYYY-MM-DD HH:MM:SS"
    std::string sourceIP;    // 来源IP地址
    std::string content;     // 新闻正文
};

// 数据读取器类
class PacketReader {
public:
    // 构造函数：传入新闻文件路径
    PacketReader();

    // 析构函数：关闭文件
    ~PacketReader();

    // 加载新闻文件
    // 参数：filePath - 新闻文件路径
    // 返回：成功返回true，失败返回false
    bool loadFile(const std::string& filePath);

    // 获取下一条新闻
    // 参数：packet - 输出参数，存储新闻数据
    // 返回：成功返回true，没有更多返回false
    bool getNextNews(NewsPacket& packet);

    // 重置读取位置（从头开始读）
    void reset();

    // 获取新闻总数
    // 返回：总新闻条数
    int getTotalCount() const;

    // 获取已读取的新闻数量
    // 返回：当前已读取条数
    int getReadCount() const;

    // 是否还有更多新闻
    // 返回：有返回true，否则false
    bool hasNext() const;

    // 获取所有新闻（一次性加载到内存）
    // 返回：所有新闻的列表
    std::vector<NewsPacket> getAllNews();

private:
    std::vector<NewsPacket> m_newsList;   // 存储所有新闻
    int m_currentIndex;                    // 当前读取位置
};

#endif // PACKET_READER_H

