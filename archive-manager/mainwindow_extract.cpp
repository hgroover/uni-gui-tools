#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plugin.h"

#include <QtDebug>
#include <QProcess>

QProcess *MainWindow::prepareRunner(bool post)
{
    QProcess *p = new QProcess(this);
    if (post)
    {
        connect( p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_PostFinished(int,QProcess::ExitStatus)) );
        connect( p, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(on_PostError(QProcess::ProcessError)) );
    }
    else
    {
        connect( p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_ExtractionFinished(int,QProcess::ExitStatus)));
        connect( p, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(on_ExtractionError(QProcess::ProcessError)));
    }
    connect( p, SIGNAL(readyRead()), this, SLOT(on_ExtractionReadyRead()));
    return p;
}

void MainWindow::on_ExtractionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo().noquote() << "Finished code" << exitCode << "status" << exitStatus;
    // FIXME need QProcess * so we can deleteLater()
    QProcess* get = qobject_cast<QProcess*>(sender());
    if (get != nullptr)
    {
        qDebug() << "Deferred delete" << get;
        QTimer::singleShot(1000, get, SLOT(deleteLater()));
    }
    else
    {
        qWarning() << "Failed to get sender";
    }
    // Get any post-processing plugins
    QList<Plugin*> postPlugins = getPluginsForContext("post");
    QString appCurrentDir(QDir::currentPath());
    int postCount = 0;
    for (int n = 0; n < postPlugins.count(); n++)
    {
        if (!postPlugins.at(n)->isValid()) continue;
        qInfo().noquote() << "Plugin[" << n << "]" << postPlugins.at(n)->id();
        QProcess * pExtract = prepareRunner(true);
        if (postCount == 0)
        {
            QDir::setCurrent(curLogExtractDir_);
        }
        postCount++;
        QString gitPath(shellCmdLine(postPlugins.at(n)->getScriptPath(), QStringList()));
        pExtract->start(gitPath);
    }
    if (postCount > 0)
    {
        QDir::setCurrent(appCurrentDir);
    }
    // Refresh regardless of exit code
    refreshCurrent();
}

void MainWindow::refreshCurrent()
{
    if (curLogFilename_.isEmpty()) return;
    QModelIndex i = logList_->findByFile(curLogFilename_);
    qInfo().noquote() << "index" << i << "for" << curLogFilename_;
    on_lstLogsModel_clicked(i);
}

void MainWindow::on_ExtractionReadyRead()
{
    QProcess *script = qobject_cast<QProcess*>(sender());
    if (nullptr == script)
    {
        qWarning() << "No sender";
        return;
    }
    qInfo().noquote() << script->readAll();
}

void MainWindow::on_ExtractionError(QProcess::ProcessError error)
{
    qWarning() << "Failed extract:" << error;
    ui->txtItemDetails->setText("Failed to extract.");
}

void MainWindow::on_PostFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo().noquote() << "Finished post code" << exitCode << "status" << exitStatus;
    // FIXME need QProcess * so we can deleteLater()
    QProcess* get = qobject_cast<QProcess*>(sender());
    if (get != nullptr)
    {
        qDebug() << "Deferred delete" << get;
        QTimer::singleShot(1000, get, SLOT(deleteLater()));
    }
    else
    {
        qWarning() << "Failed to get sender";
    }
    // Refresh regardless of exit code
    refreshCurrent();
}

void MainWindow::on_PostError(QProcess::ProcessError error)
{
    qWarning() << "Failed post-processing action:" << error;
}
