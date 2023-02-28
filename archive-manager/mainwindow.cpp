#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgconfig.h"
#include "archive-manager-globals.h"

#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    extractionValid_(false),
    bashValid_(false),
    extractScriptValid_(false),
    logListDirty_(1)
{
    ui->setupUi(this);
    MYQSETTINGS(settings);
    logDir_ = settings.value("logDir").toString();
    logFilespec_ = settings.value("logFilespec").toString();
    ui->txtLogDir->setText(logDir_);
    ui->txtFilespec->setText(logFilespec_);
    on_GitDirChanged(settings.value("gitDir", "c:/Program Files/Git").toString());
    //gitDir_ = settings.value("gitDir");
    //ui->txtGitDir->setText(gitDir_);
    on_ExtractionDirChanged(settings.value("extractionDir").toString());
    on_ExtractScriptChanged(settings.value("extractScript").toString());
    QTimer::singleShot(500, this, SLOT(on_Refresh()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qInfo().noquote() << "Testing";
    QString gitPath("\"");
    gitPath += gitShellPath_;
    gitPath += "\" -c ./foo.sh";
    int bashRes = QProcess::execute(gitPath);
    // execute() returns -1 if crash, -2 if cannot start, else ok
    switch (bashRes)
    {
    case -1:
        qCritical() << "Program crashed";
        break;
    case -2:
        qCritical().noquote() << "Failed to start" << gitPath;
        break;
    default:
        qInfo().noquote() << "Result of" << gitPath << ":" << bashRes;
        break;
    }
}

void MainWindow::on_btnQuit_clicked()
{
    close();
}

void MainWindow::on_btnConfigure_clicked()
{
    dlgConfig dlg(this);
    connect( &dlg, SIGNAL(updatedLogDir(QString)), this, SLOT(on_LogDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedLogFilespec(QString)), this, SLOT(on_LogFilespecChanged(QString)));
    connect( &dlg, SIGNAL(updatedGitDir(QString)), this, SLOT(on_GitDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedExtractionDir(QString)), this, SLOT(on_ExtractionDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedExtractScript(QString)), this, SLOT(on_ExtractScriptChanged(QString)));
    dlg.setGitDir(gitDir_);
    dlg.setLogDir(logDir_);
    dlg.setLogFilespec(logFilespec_);
    dlg.setExtractionDir(extractDir_);
    dlg.setExtractScript(extractScript_);
    int dlgRes = dlg.exec();
    disconnect( &dlg, SIGNAL(updatedLogDir(QString)), this, SLOT(on_LogDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedLogFilespec(QString)), this, SLOT(on_LogFilespecChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedGitDir(QString)), this, SLOT(on_GitDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedExtractionDir(QString)), this, SLOT(on_ExtractionDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedExtractScript(QString)), this, SLOT(on_ExtractScriptChanged(QString)));
    qInfo() << "config returned" << dlgRes;
}

void MainWindow::on_LogDirChanged(QString logDir)
{
    logDir_ = logDir;
    logListDirty_ = 1;
    QTimer::singleShot(500, this, SLOT(on_Refresh()));
    qInfo().noquote() << "Dirty dir" << logDir;
}

void MainWindow::on_LogFilespecChanged(QString logFilespec)
{
    logFilespec_ = logFilespec;
    logListDirty_ = 1;
    QTimer::singleShot(500, this, SLOT(on_Refresh()));
    qInfo().noquote() << "Dirty spec" << logFilespec;
}

void MainWindow::on_GitDirChanged(QString gitDir)
{
    gitDir_ = gitDir;
    gitBinDir_ = gitDir + "/bin";
    gitShellPath_ = gitBinDir_ + "/bash.exe";
    ui->txtGitDir->setText(gitDir);
    QFileInfo fi(gitShellPath_);
    bashValid_ = fi.isExecutable();
    if (!bashValid_)
    {
        qWarning().noquote() << "Not readable:" << gitShellPath_ << " | " << fi.fileName();
    }
}

void MainWindow::on_ExtractionDirChanged(QString extractionDir)
{
    extractDir_ = extractionDir;
    ui->txtExtractionDir->setText(extractionDir);
    QDir d(extractionDir);
    extractionValid_ = d.exists();
}

void MainWindow::on_ExtractScriptChanged(QString extractScript)
{
    extractScript_ = extractScript;
    ui->txtExtractScript->setText(extractScript);
    QFileInfo fi(extractScript);
    extractScriptValid_ = fi.exists();
}

void MainWindow::on_btnRefresh_clicked()
{
    // Force refresh
    logListDirty_ = 1;
    QTimer::singleShot(0, this, SLOT(on_Refresh()));
}

void MainWindow::on_Refresh()
{
    if (logListDirty_ == 0)
    {
        return;
    }
    logListDirty_ = 0;
    qInfo().noquote() << "Refresh" << logDir_;
    QDir dirLogs(logDir_, logFilespec_, QDir::SortFlags(QDir::Name|QDir::IgnoreCase), QDir::Files);
    QStringList a = dirLogs.entryList();
    qInfo().noquote() << "Found" << a.count() << "filespec:" << logFilespec_ << ":" << a;
    ui->lstLogs->clear();
    for (int n = 0; n < a.count(); n++)
    {
        ui->lstLogs->addItem(a[n]);
    }
}

void MainWindow::on_lstLogs_currentRowChanged(int currentRow)
{
    qDebug() << "New row" << currentRow << "extract" << extractionValid_ << "bash" << bashValid_;
    if (currentRow < 0 || currentRow >= ui->lstLogs->count())
    {
        ui->btnLogExtract->setEnabled(false);
        ui->txtItemDetails->setText("<h2>No item selected</h2>");
    }
    else
    {
        ui->btnLogExtract->setEnabled(extractionValid_ && bashValid_ && extractScriptValid_);
    }
}

void MainWindow::on_btnLogExtract_clicked()
{
    QFileInfo fi(ui->lstLogs->currentItem()->text());
    qInfo().noquote() << "Extracting" << fi.fileName() << "base" << fi.baseName();
}
