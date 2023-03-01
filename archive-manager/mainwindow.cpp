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
    curSelectedRow_ = currentRow;
    if (currentRow < 0 || currentRow >= ui->lstLogs->count())
    {
        ui->btnLogExtract->setEnabled(false);
        ui->btnView->setEnabled(false);
        ui->btnClear->setEnabled(false);
        curLogExtractDir_.clear();
        curLogFilename_.clear();
        curLogTarballPath_.clear();
        ui->txtItemDetails->setText("<h2>No item selected</h2>");
    }
    else
    {
        curLogFilename_ = ui->lstLogs->currentItem()->text();
        //QFileInfo fi(curLogFilename_);
        QString baseName(tarballBasename(curLogFilename_));
        QDir emptyDir(extractDir_ + '/' + baseName);
        curLogTarballPath_ = logDir_ + '/' + curLogFilename_;
        curLogExtractDir_ = emptyDir.path();
        ui->btnLogExtract->setEnabled(extractionValid_ && bashValid_ && extractScriptValid_ && !emptyDir.exists());
        ui->btnClear->setEnabled(emptyDir.exists());
        ui->btnView->setEnabled(emptyDir.exists());
        populateViewBrowser(emptyDir.exists());
    }
}

void MainWindow::on_btnLogExtract_clicked()
{
    QDir parentDir(extractDir_);
    if (parentDir.mkpath(curLogExtractDir_))
    {
        qInfo().noquote() << "Extracting" << curLogFilename_ << "to" << curLogExtractDir_;
        QProcess pExtract(this);
        // You would think this would work, but the script will still run in the app dir unless we explicitly change
        // the app's working directory.
        pExtract.setWorkingDirectory(curLogExtractDir_);
        QString appCurrentDir(QDir::currentPath());
        QDir::setCurrent(curLogExtractDir_);
        QString gitPath("\"");
        gitPath += gitShellPath_;
        gitPath += "\" -c \"";
        gitPath += bashify(extractScript_);
        gitPath += ' ';
        gitPath += bashify(curLogTarballPath_);
        gitPath += '\"';
        int res = pExtract.execute(gitPath);
        // execute() returns -1 if crash, -2 if cannot start, else ok
        switch (res)
        {
        case -1:
            qCritical() << "Program crashed";
            break;
        case -2:
            qCritical().noquote() << "Failed to start" << gitPath;
            break;
        case 0:
            qInfo().noquote() << "Success, cwd" << pExtract.workingDirectory();
            on_lstLogs_currentRowChanged(ui->lstLogs->currentRow());
            break;
        default:
            qInfo().noquote() << "Script failed - result of" << gitPath << ":" << res;
            break;
        }
        QDir::setCurrent(appCurrentDir);
    }
    else
    {
        qInfo().noquote() << "Failed to create dir path" << curLogExtractDir_;
    }
    //qInfo().noquote() << "Extracting" << fi.fileName() << "base" << tarballBasename(fi.fileName());
}

QString MainWindow::tarballBasename(QString tarballFilename)
{
    // Trailing .tar, tar.bz2, .tgz
    static const char *szExt[] = {".tar.bz2", ".tar", ".tgz"};
    for (size_t n = 0; n < sizeof(szExt)/sizeof(szExt[0]); n++)
    {
        int extPos = tarballFilename.lastIndexOf(szExt[n], -1, Qt::CaseInsensitive);
        if (extPos > 0)
        {
            return tarballFilename.left(extPos);
        }
    }
    // Not found
    qWarning().noquote() << "No extension found in" << tarballFilename;
    return tarballFilename;
}

QString MainWindow::bashify(QString windowsPath)
{
    QString driveLetter = windowsPath.left(2);
    if (driveLetter.length() < 2 || driveLetter[1] != ':')
    {
        qWarning().noquote() << "Not an absolute windows path:" << windowsPath;
        return windowsPath;
    }
    return driveLetter.left(1).toLower().prepend('/') + windowsPath.mid(2);
}

void MainWindow::on_btnView_clicked()
{
    QDir logDir(curLogExtractDir_);
    logDir.setFilter(QDir::NoDotAndDotDot|QDir::AllEntries);
    qInfo().noquote() << "Contents of" << logDir.path() << "count:" << logDir.count();
    QFileInfoList a(logDir.entryInfoList());
    for (int n = 0; n < a.length(); n++)
    {
        qInfo().noquote() << '[' << n << ']' << a[n];
    }
}

void MainWindow::on_btnClear_clicked()
{
    QDir parentDir(extractDir_);
    if (!curLogExtractDir_.isEmpty()
            && parentDir.exists(curLogExtractDir_))
    {
        qInfo().noquote() << "Removing" << curLogExtractDir_;
        QDir exDir(curLogExtractDir_);
        if (exDir.removeRecursively())
        {
            qInfo().noquote() << "Success";
            on_lstLogs_currentRowChanged(ui->lstLogs->currentRow());
        }
        else
        {
            qWarning().noquote() << "Failed to remove recursively";
        }
    }
    else
    {
        qWarning().noquote() << "Cannot remove" << curLogExtractDir_;
    }
}

void MainWindow::populateViewBrowser(bool extracted)
{
    QString html;
    if (extracted)
    {
        html = "<h5>" + curLogExtractDir_ + "</h5>";
        html += "<p><a href=\"hello\">Test link</a></p>";
    }
    else
    {
        html = "<h4>No contents</h4>";
    }
    ui->txtItemDetails->setText(html);
}

void MainWindow::on_txtItemDetails_anchorClicked(const QUrl &arg1)
{
    qInfo().noquote() << "url:" << arg1.toString();
}
