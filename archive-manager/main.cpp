#include "mainwindow.h"
#include <QApplication>

#include <QtGlobal>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QMessageLogContext>

#include <iostream>

static int g_verbose = 1;

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
        fprintf( stderr, "%s\n", localMsg.constData() );
        break;
    case QtFatalMsg:
        //fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationDisplayName("Archive Manager");
    a.setApplicationVersion("1.0.1");
    // Note that qInfo(), qWarning(), qCritical() etc. do not go to stdout / stderr
    // This will be visible on console only if started from git-bash - windows output requires using console api
    //std::cout << "Hello from main" << std::endl;
    //qWarning().noquote() << "Starting up" << argv[0];
    //qInfo().noquote() << "Using qInfo";
    w.show();

    return a.exec();
}
