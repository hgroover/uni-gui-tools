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
    // Calling processEvents() here is a hack.
    // Ideally we restructure init to set up a list of expected returns, then when we either get all
    // or a timeout occurs, perform the final summary on initPlugins()
    QCoreApplication::processEvents();
    if (!defRunner.waitForStarted(5000))
    {
        curState_ = DS_BADDEFS;
        qCritical().noquote() << "Failed to start" << scriptPath;
        deleteLater();
        return;
    }
    QCoreApplication::processEvents();
    if (!defRunner.waitForFinished(10000))
    {
        curState_ = DS_BADDEFS;
        qCritical().noquote() << "Failed to get defs" << scriptPath << "state" << defRunner.state();
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

bool Plugin::runScript(const QList<QString> &args, const QMap<QString, QString> &env)
{
    MainWindow *m = qobject_cast<MainWindow*>(parent());
    QString cmd(m->shellCmdLine(scriptPath_, args));
    QProcess * runner = new QProcess(this);
    QProcessEnvironment processEnv;
    if (!env.isEmpty())
    {
        processEnv = QProcessEnvironment::systemEnvironment();
        QMap<QString,QString>::const_iterator i;
        for (i = env.begin(); i != env.end(); i++)
        {
            processEnv.insert(i.key(), i.value());
        }
        qDebug().noquote() << "Set process environment" << env;
        runner->setProcessEnvironment(processEnv);
    }
    else
    {
        qDebug().noquote() << "No environment";
    }
    connect( runner, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(on_errorOccurred(QProcess::ProcessError)));
    connect( runner, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_finished(int,QProcess::ExitStatus)));
    connect( runner, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    qDebug().noquote() << "Starting" << cmd;
    runner->start(cmd);
    if (!runner->waitForStarted(500))
    {
        qCritical().noquote() << "Failed to start" << cmd;
        QTimer::singleShot(1000, runner, SLOT(deleteLater()));
        return false;
    }
    return true;
}

void Plugin::on_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit outputLine(QString().sprintf("Exit code %d", exitCode));
    QProcess *r = qobject_cast<QProcess*>(sender());
    //_getOutput(r);
    QTimer::singleShot(1000, r, SLOT(deleteLater()));
}

void Plugin::on_errorOccurred(QProcess::ProcessError error)
{
    emit outputLine("Error: " + error);
    QProcess *r =  qobject_cast<QProcess*>(sender());
    QTimer::singleShot(1000, r, SLOT(deleteLater()));
}

void Plugin::on_readyRead()
{
    QProcess *r = qobject_cast<QProcess*>(sender());
    _getOutput(r);
}

void Plugin::_getOutput(QProcess *r)
{
    QList<QByteArray> b = r->readAll().split('\n');
    for (int n = 0; n < b.count(); n++)
    {
        if (b[n].isEmpty()) continue;
        emit outputLine(b[n]);
    }
}
