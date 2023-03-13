#include "mainwindow.h"
#include "archive-manager-globals.h"
#include <QApplication>

#include <QtGlobal>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QMessageLogContext>
#include <QDir>
#include <QFile>

#include <iostream>
#include <stdio.h>

static int g_verbose = 1;
FILE *fLog = nullptr;
QFile logFile;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    Q_UNUSED(context);
    // For release build, context.file and context.function will be null and context.line will always be 0
    switch (type) {
    case QtDebugMsg:
        //fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        if (0 == g_verbose) break;
        // else fall through
        //[[clang::fallthrough]];
    case QtInfoMsg:
        //fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //break;
    case QtWarningMsg:
        //fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //break;
    case QtCriticalMsg:
        //fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //break;
        fprintf( fLog, "%s\n", localMsg.constData() );
        fflush( fLog );
        break;
    case QtFatalMsg:
        fprintf( fLog, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char *argv[])
{
    QDir appDir(QApplication::applicationDirPath());
    QString prevLogPath(appDir.filePath("archive-manager-previous.log"));
    QString curLogPath(appDir.filePath("archive-manager.log"));
    QFile prevLog(prevLogPath);
    if (prevLog.exists()) prevLog.remove();
    logFile.setFileName(curLogPath);
    if (logFile.exists())
    {
        logFile.rename(prevLogPath);
    }
    fLog = fopen( curLogPath.toLocal8Bit().constData(), "w" );
    if (nullptr == fLog)
    {
        qWarning().noquote() << "Failed to open" << logFile.fileName();
        fLog = stderr;
    }
    qInstallMessageHandler(myMessageOutput);
    qInfo().noquote() << "Starting" << QApplication::applicationName();
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationDisplayName("Archive Manager");
    a.setApplicationVersion(MY_APP_VER);
    // Note that qInfo(), qWarning(), qCritical() etc. do not go to stdout / stderr
    // This will be visible on console only if started from git-bash - windows output requires using console api
    //std::cout << "Hello from main" << std::endl;
    //qWarning().noquote() << "Starting up" << argv[0];
    //qInfo().noquote() << "Using qInfo";
    qInfo().noquote() << "Starting archive-manager v" MY_APP_VER;
    w.show();

    return a.exec();
}
