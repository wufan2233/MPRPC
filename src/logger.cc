#include "logger.h"

// 获取单例模式
Logger &Logger::GetInsrance()
{
    static Logger logger;
    return logger;
}

// 构造函数，课后查资料，多出不理解
Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&]() { // [&]是什么意思 参数为什么是lambda ???
        for (;;)
        {
            // 获取当前日期，然后取日志信息，写入相应的日志文件中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128] = {0};
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file: " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // 不足：每次写入都需要打开关闭文件
            // 解决：当队列不为空，可以不关闭文件一直写入，为防止文件关闭不了，可以设置一定的读取上限
            std::string msg = m_lckque.Pop();

            // 每条日志的时间信息：时:分:秒
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ",
                    nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });

    // 设置分离线程，守护线程
    writeLogTask.detach(); // ???不是很理解
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志，把日志信息写入lockqueue缓存区当中
void Logger::Log(std::string msg)
{
    m_lckque.Push(msg);
}