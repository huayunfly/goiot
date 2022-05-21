#ifndef LOG_H
#define LOG_H

#endif // LOG_H

#include <QString>
#include <fstream>
#include <iostream>

// Custom log by huayunfly at 126.com
// @date 2022-05-21

namespace logger
{
class FileLog
{
public:
    FileLog(const QString& log_path);

    ~FileLog();

    FileLog(const FileLog&) = delete;

    FileLog& operator=(const FileLog&) = delete;

private:
    void InitLogger();
    static void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static std::ofstream file_;
    QString log_path_;
};

}
