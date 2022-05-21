#include <QApplication>
#include <QDir>
#include <QDateTime>
#include "log.h"

namespace logger
{

std::ofstream FileLog::file_;

FileLog::FileLog(const QString& log_path) :
    log_path_(log_path)
{
    InitLogger();
}

FileLog::~FileLog()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void FileLog::InitLogger()
{
    QString log_dir =
            QApplication::applicationDirPath() + "/" + log_path_;

    QDir dir(log_dir);
    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }
    QString time =
            QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filepath = log_dir + "/" + "log_" + time + ".txt";
    QFile file(filepath);
    if (!file.exists())
    {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.close();
    }
    // Remove the file oversize file.
    file_.open(filepath.toStdString());
    qInstallMessageHandler(MessageOutput);
}

void FileLog::MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    std::string time =
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    switch (type) {
    case QtDebugMsg:
        file_ << "[" << time << "]" << " Debug: " << localMsg.constData() << std::endl;
#ifdef QT_DEBUG
        std::cerr << localMsg.constData() << std::endl;
#endif
        break;
    case QtInfoMsg:
        file_ << "[" << time << "]" << " Info: " << std::string(localMsg) << std::endl;
#ifdef QT_DEBUG
        std::cerr << localMsg.constData() << std::endl;
#endif
        break;
    case QtWarningMsg:
        file_ << "[" << time << "]" << " Warning: " << std::string(localMsg) << std::endl;
#ifdef QT_DEBUG
        std::cerr << localMsg.constData() << std::endl;
#endif
        break;
    case QtCriticalMsg:
        file_ << "[" << time << "]" << " Critical: " << std::string(localMsg) << std::endl;
#ifdef QT_DEBUG
        std::cerr << localMsg.constData() << std::endl;
#endif
        break;
    case QtFatalMsg:
        file_ << "[" << time << "]" << " Fatal: " << std::string(localMsg) << std::endl;
#ifdef QT_DEBUG
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
#endif
        break;
    }
}

}
