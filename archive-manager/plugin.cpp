#include "plugin.h"
#include "mainwindow.h"

#include <QDebug>

Plugin::Plugin(QObject *parent) : QObject(parent)
{
    curState_ = DS_INIT;
}

void Plugin::init(QString scriptPath)
{
    MainWindow *m = qobject_cast<MainWindow*>(parent());
    QStringList args;
    args << "__getdefs";
    scriptPath_ = scriptPath;
    QString cmd(m->shellCmdLine(scriptPath, args));
    QProcess defRunner(this);
    defRunner.start(cmd);
    if (!defRunner.waitForStarted(500))
    {
        curState_ = DS_BADDEFS;
        qCritical().noquote() << "Failed to start" << scriptPath;
        deleteLater();
        return;
    }
    if (!defRunner.waitForFinished(1000))
    {
        curState_ = DS_BADDEFS;
        qCritical().noquote() << "Failed to get defs" << scriptPath;
        deleteLater();
        return;
    }
    QByteArray b(defRunner.readAll());
    QJsonParseError jsonError;
    defs_ = QJsonDocument::fromJson(b, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !defs_.isObject())
    {
        curState_ = DS_BADDEFS;
        qCritical().noquote() << "No json object" << scriptPath << "raw data" << b << "error" << jsonError.errorString();
        deleteLater();
        return;
    }

    QJsonObject j = defs_.object();
    if (j["ID"].isNull() || j["Context"].isNull())
    {
        curState_ = DS_INVALID;
        qCritical().noquote() << "Missing ID and/or Context" << scriptPath;
        deleteLater();
        return;
    }

    id_ = j["ID"].toString();
    context_ = j["Context"].toString();
    qInfo().noquote() << "ID" << id_ << "Context" << context_;
    curState_ = DS_READY;
    m->addPlugin(this);
}

QJsonObject Plugin::form()
{
    QJsonObject j = defs_.object();
    return j["Form"].toObject();
}

QByteArray Plugin::runScript(QStringList args, bool &success)
{
    success = false;
    QByteArray b;
    MainWindow *m = qobject_cast<MainWindow*>(parent());
    QString cmd(m->shellCmdLine(scriptPath_, args));
    QProcess defRunner(this);
    defRunner.start(cmd);
    if (!defRunner.waitForStarted(500))
    {
        qCritical().noquote() << "Failed to start" << cmd;
        return b;
    }
    if (!defRunner.waitForFinished(1000))
    {
        qCritical().noquote() << "Failed to finish" << cmd;
        return b;
    }
    b = defRunner.readAll();
    success = true;
    return b;
}
