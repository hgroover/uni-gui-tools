#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QProcess>

QProcess *MainWindow::prepareRunner()
{
    QProcess *p = new QProcess(this);
    connect( p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_ExtractionFinished(int,QProcess::ExitStatus)));
    connect( p, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(on_ExtractionError(QProcess::ProcessError)));
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
    // Refresh regardless of exit code
    {
        QModelIndex i = logList_->findByFile(curLogFilename_);
        qInfo().noquote() << "index" << i << "for" << curLogFilename_;
        on_lstLogsModel_clicked(i);
    }
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
