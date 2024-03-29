#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgconfig.h"
#include "archive-manager-globals.h"
#include "basicfileviewer.h"

#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QFileInfo>

int MainWindow::g_verbose = 0;
const QList<QList<QString>> MainWindow::a_sortLinks = {
    {"dateDescending", "date▼", "Show logs sorted by download date starting from most recent"},
    {"dateAscending", "date▲", "Show logs sorted by download date from oldest to newest"},
    {"nameDescending", "name▼", "Logs sorted by name in descending order"},
    {"nameAscending", "name▲", "Logs sorted by name in ascending order"}
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    web_(this),
    dlgPlugins_(this),
    extractionValid_(false),
    bashValid_(false),
    extractScriptValid_(false),
    logListDirty_(1)
{
    ui->setupUi(this);
    logList_ = new LogListModel(ui->lstLogsModel);
    ui->lstLogsModel->setModel(logList_);
    on_InitialLoad();
    MYQSETTINGS(settings);
    setWindowTitle(QApplication::applicationDisplayName() + " v" MY_APP_VER);
    restoreGeometry(settings.value(cfg_mw_geometry).toByteArray());
    restoreState(settings.value(cfg_mw_state).toByteArray());
    connect(this, SIGNAL(updatedBaseUrl(QString)), &web_, SLOT(on_UpdatedBaseUrl(QString)));
    connect(this, SIGNAL(updatedLogFilespec(QString)), &web_, SLOT(on_UpdatedFilespec(QString)));
    connect(this, SIGNAL(updatedLogDir(QString)), &web_, SLOT(on_UpdatedLogDir(QString)));
    connect(&web_, SIGNAL(downloadCompleted(QString)), this, SLOT(on_DownloadCompleted(QString)));
    connect(this, SIGNAL(pluginListReady()), &dlgPlugins_, SLOT(on_pluginListReady()));
    connect(&dlgPlugins_, SIGNAL(pluginDialogReady()), this, SLOT(on_PluginDialogReady()));
    selectLogSort("dateDescending");
    emit updatedLogDir(logDir_);
    QTimer::singleShot(500, this, SLOT(on_Refresh()));
    //QTimer::singleShot(2000, this, SLOT(testAnimator()));
    QTimer::singleShot(0, this, SLOT(loadPlugins()));
}

MainWindow::~MainWindow()
{
    disconnect(this, SIGNAL(updatedBaseUrl(QString)), &web_, SLOT(on_UpdatedBaseUrl(QString)));
    disconnect(this, SIGNAL(updatedLogFilespec(QString)), &web_, SLOT(on_UpdatedFilespec(QString)));
    disconnect(this, SIGNAL(updatedLogDir(QString)), &web_, SLOT(on_UpdatedLogDir(QString)));
    disconnect(&web_, SIGNAL(downloadCompleted(QString)), this, SLOT(on_DownloadCompleted(QString)));
    web_.close();
    logList_->deleteLater();
    logList_ = nullptr;
    for (QMap<QString,Plugin*>::iterator i = plugins_.begin(); i != plugins_.end(); i++)
    {
        i.value()->deleteLater();
    }
    delete ui;
}

void MainWindow::on_btnQuit_clicked()
{
    MYQSETTINGS(settings);
    settings.setValue(cfg_mw_geometry, saveGeometry());
    settings.setValue(cfg_mw_state, saveState());
    // This works as expected on a single-monitor setup
    qDebug().noquote() << "Geometry" << saveGeometry().toHex();
    // No difference, as expected
    //qDebug().noquote() << "Superclass" << static_cast<QMainWindow*>(this)->saveGeometry().toHex();
    qDebug().noquote() << "Title bar height" << QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    close();
}

void MainWindow::on_InitialLoad()
{
    // Load vars from settings into member vars
    MYQSETTINGS(settings);
    // Both of these need to get set without refreshing the log list
    logDir_ = settings.value(cfg_logDir).toString();
    logFilespec_ = settings.value(cfg_logFilespec).toString();
    ui->txtLogDir->setText(logDir_);
    ui->txtFilespec->setText(logFilespec_);
    on_GitDirChanged(settings.value(cfg_gitDir, "c:/Program Files/Git").toString());
    on_ExtractionDirChanged(settings.value(cfg_extractionDir).toString());
    on_ExtractScriptChanged(settings.value(cfg_extractionScript).toString());
    on_BaseUrlChanged(settings.value(cfg_baseUrl).toString());
    on_ViewExternalSizeTrigger(settings.value(cfg_viewSizeTrigger).toInt());
    on_ViewExternalViewer(settings.value(cfg_viewExternalViewer).toString());
}

void MainWindow::on_btnConfigure_clicked()
{
    dlgConfig dlg(this);
    dlg.loadValues();
    // These need to be echoed in initial load
    connect( &dlg, SIGNAL(updatedLogDir(QString)), this, SLOT(on_LogDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedLogFilespec(QString)), this, SLOT(on_LogFilespecChanged(QString)));
    connect( &dlg, SIGNAL(updatedGitDir(QString)), this, SLOT(on_GitDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedExtractionDir(QString)), this, SLOT(on_ExtractionDirChanged(QString)));
    connect( &dlg, SIGNAL(updatedExtractScript(QString)), this, SLOT(on_ExtractScriptChanged(QString)));
    connect( &dlg, SIGNAL(updatedBaseUrl(QString)), this, SLOT(on_BaseUrlChanged(QString)));
    connect( &dlg, SIGNAL(updatedViewExternalViewerTrigger(int)), this, SLOT(on_ViewExternalSizeTrigger(int)));
    connect( &dlg, SIGNAL(updatedViewExternalViewer(QString)), this, SLOT(on_ViewExternalViewer(QString)));
    int dlgRes = dlg.exec();
    disconnect( &dlg, SIGNAL(updatedLogDir(QString)), this, SLOT(on_LogDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedLogFilespec(QString)), this, SLOT(on_LogFilespecChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedGitDir(QString)), this, SLOT(on_GitDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedExtractionDir(QString)), this, SLOT(on_ExtractionDirChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedExtractScript(QString)), this, SLOT(on_ExtractScriptChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedBaseUrl(QString)), this, SLOT(on_BaseUrlChanged(QString)));
    disconnect( &dlg, SIGNAL(updatedViewExternalViewerTrigger(int)), this, SLOT(on_ViewExternalSizeTrigger(int)));
    disconnect( &dlg, SIGNAL(updatedViewExternalViewer(QString)), this, SLOT(on_ViewExternalViewer(QString)));
    qInfo() << "config returned" << dlgRes;
}

void MainWindow::on_LogDirChanged(QString logDir)
{
    logDir_ = logDir;
    logListDirty_ = 1;
    ui->txtLogDir->setText(logDir);
    QTimer::singleShot(500, this, SLOT(on_Refresh()));
    emit updatedLogDir(logDir);
    qInfo().noquote() << "Dirty dir" << logDir;
}

void MainWindow::on_LogFilespecChanged(QString logFilespec)
{
    logFilespec_ = logFilespec;
    logListDirty_ = 1;
    ui->txtFilespec->setText(logFilespec);
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

void MainWindow::on_BaseUrlChanged(QString baseUrl)
{
    baseUrl_ = baseUrl;
    ui->txtBaseUrl->setText(baseUrl);
    ui->btnDownload->setEnabled(!baseUrl.isEmpty());
    emit updatedBaseUrl(baseUrl);
}

void MainWindow::on_ViewExternalSizeTrigger(int fileSize)
{
    viewExternalViewTrigger_ = fileSize;
}

void MainWindow::on_ViewExternalViewer(QString viewerApp)
{
    viewExternalViewerApp_ = viewerApp;
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
    qInfo().noquote() << "Found" << a.count() << "filespec:" << logFilespec_; // << ":" << a;
    QFileInfoList ai = dirLogs.entryInfoList();
    // Does not work with Qt before 5.15
    //qInfo().noquote() << "Info list" << ai;
    logList_->setFileInfoList(ai);
    /**
    for (int n = 0; n < a.count(); n++)
    {
        //ui->lstLogs->addItem(a[n]);
        //qDebug().noquote() << '[' << n << ']' << ai[n].fileName() << a[n];
        SortableListWidgetItem * sit = new SortableListWidgetItem(ai[n].fileName(), ui->lstLogs);
        sit->setFileInfo(ai[n]);
        ui->lstLogs->addItem(sit);
    }
    **/
    emit updatedLogFilespec(logFilespec_);
}

bool MainWindow::canShell() const
{
#if defined(Q_OS_WIN)
    return bashValid_;
#else
    return true;
#endif
}

QString MainWindow::shellCmdLine(QString shellPath, QStringList shellArgs)
{
    QString cmdLine;
#if defined(Q_OS_WIN)
    cmdLine = "\"";
    cmdLine += gitShellPath_;
    cmdLine += "\" -c \"";
    cmdLine += bashify(shellPath);
    for (int n = 0; n < shellArgs.size(); n++)
    {
        cmdLine += ' ';
        cmdLine += bashify(shellArgs[n]);
    }
    cmdLine += '"';
#else
        // Linux / MAC_OS
    cmdLine = shellPath;
    for (int n = 0; n < shellArgs.size(); n++)
    {
        cmdLine += " \"";
        cmdLine += shellArgs[n];
        cmdLine += '"';
    }
#endif
    return cmdLine;
}

void MainWindow::on_btnLogExtract_clicked()
{
    QDir parentDir(extractDir_);
    if (parentDir.mkpath(curLogExtractDir_))
    {
        ui->btnLogExtract->setEnabled(false);
        ui->txtItemDetails->setText("<html><p>Please wait... extraction in progress<p></html>");
        ui->statusBar->showMessage("Extracting " + curLogFilename_, 10000);
        QApplication::processEvents();
        QApplication::processEvents();
        QApplication::processEvents();
        qInfo().noquote() << "Extracting" << curLogFilename_ << "to" << curLogExtractDir_;
        QProcess * pExtract = prepareRunner(false);
        // You would think this would work, but the script will still run in the app dir unless we explicitly change
        // the app's working directory.
        pExtract->setWorkingDirectory(curLogExtractDir_);
        QString appCurrentDir(QDir::currentPath());
        QDir::setCurrent(curLogExtractDir_);
        QString gitPath(shellCmdLine(extractScript_, QStringList() << curLogTarballPath_));
        pExtract->start(gitPath);
        // execute() returns -1 if crash, -2 if cannot start, else ok
        // start will only return on signal
        /****
        switch (res)
        {
        case -1:
            qCritical() << "Program crashed";
            ui->txtItemDetails->setText("Critical error while extracting");
            break;
        case -2:
            qCritical().noquote() << "Failed to start" << gitPath;
            ui->txtItemDetails->setText("Failed to run extraction script");
            break;
        case 0:
            qInfo().noquote() << "Started, cwd" << pExtract->workingDirectory();
            break;
        default:
            qInfo().noquote() << "Script failed - result of" << gitPath << ":" << res;
            ui->txtItemDetails->setText("Failed to extract. current git path:" + gitPath);
            break;
        }
        ***/
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
        qDebug().noquote() << "Not an absolute windows path:" << windowsPath;
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
        // Qt does not support QFileInfo output before 5.15
        qInfo().noquote() << '[' << n << ']' << a[n].absoluteFilePath();
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
            //FIXME on_lstLogs_currentRowChanged(ui->lstLogs->currentRow());
            ui->statusBar->showMessage("Cleared directory successfully", 10000);
        }
        else
        {
            qWarning().noquote() << "Failed to remove recursively";
            ui->statusBar->showMessage("Failed to clear directory", 10000);
        }
        refreshCurrent();
    }
    else
    {
        qWarning().noquote() << "Cannot remove" << curLogExtractDir_;
    }
}

void MainWindow::on_txtItemDetails_anchorClicked(const QUrl &arg1)
{
    // Get verb:object where object may also have : (e.g. windows paths)
    QString s(arg1.toString());
    int colonPos = s.indexOf(':');
    if (colonPos <= 0)
    {
        qWarning().noquote() << "Invalid verb:object" << s;
        return;
    }
    QString verb(s.left(colonPos));
    QString verbObject(s.mid(colonPos+1));
    qInfo().noquote() << "Performing" << verb << "on" << verbObject; // << "with" << curLogExtractDir_ << "filename" << curLogFilename_;
    if (verb == "extract")
    {
        qInfo().noquote() << "Extracting" << verbObject << "curLogFilename_" << curLogFilename_;
        QTimer::singleShot(0, this, SLOT(on_btnLogExtract_clicked()));
    }
    else if (verb == "view")
    {
        QDir d(curLogExtractDir_);
        QFileInfo fi(d.filePath(verbObject));
        if (viewExternalViewTrigger_ >= 0 && fi.size() >= viewExternalViewTrigger_ && viewExternalViewerApp_.length() > 0)
        {
            qInfo().noquote() << "Size triggers external viewer:" << fi.size() << "bytes in" << verbObject;
            int res = QProcess::execute('"' + viewExternalViewerApp_ + "\" \"" + fi.filePath() + '"');
            qInfo().noquote() << "Execute returned" << res;
        }
        else
        {
            qInfo().noquote() << "Size" << fi.size() << "for" << fi.filePath() << "under limit" << viewExternalViewTrigger_;
            BasicFileViewer *view = new BasicFileViewer(nullptr, curLogExtractDir_, verbObject);
            view->setModal(false);
            view->showNormal();
            setFocus();
        }
    }
    else
    {
        qWarning().noquote() << "Don't know how to" << verb << verbObject;
    }
}

void MainWindow::on_btnDownload_clicked()
{
    if (!web_.isVisible())
    {
        web_.setVisible(true);
    }
    qInfo().noquote() << "Showing download window with" << baseUrl_;
    web_.show();
    emit updatedBaseUrl(baseUrl_);
    web_.setFocus();
}

void MainWindow::on_DownloadCompleted(QString filename)
{
    qInfo().noquote() << "Download completed" << filename;
    // filename should exist in downloads
    //SortableListWidgetItem *sit = new SortableListWidgetItem(filename, ui->lstLogs);
    QFileInfo fi;
    fi.setFile(QDir(logDir_), filename);
    //sit->setFileInfo(fi);
    //ui->lstLogs->addItem(sit);
    //ui->lstLogs->setCurrentItem(sit);
    logList_->addFileInfo(fi);
    //QTimer::singleShot(100, this, SLOT(on_AssertFirstSelection()));
    //qInfo() << "deferred reselect";
    QTimer::singleShot(100, this, [this](){
        QModelIndex last(logList_->lastAdded());
        qDebug().noquote() << "lambda:" << last;
        ui->lstLogsModel->selectionModel()->select(last, QItemSelectionModel::SelectCurrent);
        on_lstLogsModel_clicked(last);
    });
}

void MainWindow::testAnimator()
{
    static int counter = 0;
    int rowSelection = counter % 12;
    bool withClick = (counter >= 12);
    QModelIndex test = logList_->index(rowSelection, 0);
    ui->lstLogsModel->selectionModel()->select(test, QItemSelectionModel::SelectCurrent);
    if (withClick) on_lstLogsModel_clicked(test);
    counter++;
    if (counter < 30)
    {
        qDebug() << "testAnimator finished" << counter;
        QTimer::singleShot(5000, this, SLOT(testAnimator()));
    }
    else
    {
        qDebug() << "testAnimator finished";
    }
}

// Used as a target for singleshot
void MainWindow::on_AssertFirstSelection()
{
    qInfo().noquote() << "Asserting selection of row 0";
    //ui->lstLogs->setCurrentRow(0); // This will fire the event handler
}

bool MainWindow::hasLocalCopy(QString logFile)
{
    QModelIndex i = logList_->findByFile(logFile);
    return i.isValid();
}

void MainWindow::on_lblGlobalOptions_linkActivated(const QString &link)
{
    if (link == "toggleVerbose")
    {
        g_verbose = (g_verbose + 1) % 2;
        qInfo().noquote() << "Verbose" << g_verbose;
        ui->statusBar->showMessage(QString().sprintf("Debug level set to %d", g_verbose), 10000);
    }
    else if (link.length() > 0)
    {
        qWarning().noquote() << "Invalid link" << link;
    }
}

void MainWindow::on_lblGlobalOptions_linkHovered(const QString &link)
{
    if (link.length() == 0) return;
    QString msg("Unknown global option link ");
    msg += link;
    if (link == "toggleVerbose")
    {
        msg = QString().sprintf("Current verbose %d - click to change", g_verbose);
    }
    ui->statusBar->showMessage(msg, 10000);
}

void MainWindow::on_lblSortOptions_linkActivated(const QString &link)
{
    if (link.isEmpty()) return;
    qDebug().noquote() << "sort:" << link;
    logList_->on_sortOrder(link);
    selectLogSort(link);
}

void MainWindow::on_lblSortOptions_linkHovered(const QString &link)
{
    if (link.isEmpty()) return;
    qDebug().noquote() << "hover:" << link;
    QString msg("Unknown link ");
    msg += link;
    for (int n = 0; n < a_sortLinks.size(); n++)
    {
        if (link == a_sortLinks[n][0])
        {
            msg = a_sortLinks[n][2];
            break;
        }
    }
    ui->statusBar->showMessage(msg, 15000);
}

void MainWindow::selectLogSort(QString sortOption)
{
    QString html("<html><head/><body><p>Sort options: ");
    for (int n = 0; n < a_sortLinks.size(); n++)
    {
        bool selected = (sortOption == a_sortLinks[n][0]);
        if (n > 0) html += " | ";
        if (selected)
        {
            html += "[ ";
            html += a_sortLinks[n][1];
            html += " ] ";
        }
        else
        {
            html += " <a href=\"";
            html += a_sortLinks[n][0];
            html += "\">";
            html += a_sortLinks[n][1];
            html += "</a> ";
        }
    }
    //"[ date▼ ] | <a href="dateAscending">date▲</a></p></body></html>")
    html += "</html>";
    qDebug().noquote() << "sort option" << sortOption << "result html" << html;
    ui->lblSortOptions->setText(html);
    //SortableListWidgetItem::setSortOption(sortOption);
    //ui->lstLogs->sortItems();
}

void MainWindow::on_lstLogsModel_clicked(const QModelIndex &index)
{
    // Changed selection
    if (!index.isValid())
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
        curLogFilename_ = logList_->fileFromIndex(index);
        qInfo().noquote() << "New log model selection" << curLogFilename_ << "ev" << extractionValid_ << "bv" << canShell() << "esv" << extractScriptValid_;
        ui->statusBar->showMessage("Selected " + curLogFilename_, 10000);
        //QFileInfo fi(curLogFilename_);
        QString baseName(tarballBasename(curLogFilename_));
        QDir emptyDir(extractDir_ + '/' + baseName);
        curLogTarballPath_ = logDir_ + '/' + curLogFilename_;
        curLogExtractDir_ = emptyDir.path();
        ui->btnLogExtract->setEnabled(extractionValid_ && canShell() && extractScriptValid_ && !emptyDir.exists());
        ui->btnClear->setEnabled(emptyDir.exists());
        ui->btnView->setEnabled(emptyDir.exists());
        qInfo().noquote() << "Updating view browser, dir exists:" << emptyDir.exists();
        populateViewBrowser(emptyDir.exists());
    }
}

void MainWindow::addPlugin(Plugin *p)
{
    if (!p->isValid()) return;
    QString id(p->id());
    if (plugins_.contains(id))
    {
        qWarning().noquote() << "Replacing" << id;
        if (plugins_[id] != p) plugins_[id]->deleteLater();
    }
    else
    {
        qInfo().noquote() << "Adding plugin" << id << "with context" << p->context();
    }
    plugins_[id] = p;
}

void MainWindow::removePlugin(Plugin *p)
{
    QString id(p->id());
    if (plugins_.contains(id))
    {
        qInfo().noquote() << "Removing" << id;
        plugins_.remove(id);
    }
    else
    {
        qWarning().noquote() << "Not in map:" << id;
    }
    p->deleteLater();
}

int MainWindow::loadPlugins()
{
    QDir pluginDir(QDir(QApplication::applicationDirPath()).absoluteFilePath("am-plugins"));
    QFileInfoList pluginList(pluginDir.entryInfoList(QStringList() << "*.am-plugin", QDir::Files));
    int totalAdded = 0;
    qDebug().noquote() << "Reading from" << pluginDir.absolutePath();
    qDebug().noquote() << "Info list size:" << pluginList.size();
    for (int n = 0; n < pluginList.size(); n++)
    {
        if (!pluginList[n].isFile()) continue;
        Plugin *p = new Plugin(this);
        //qDebug().noquote() << "Plugin:" << pluginList[n].filePath();
        p->init(pluginList[n].filePath());
        if (p->isValid())
        {
            totalAdded++;
        }
    }
    qInfo() << totalAdded << "plugins added";
    emit pluginListReady();
    return totalAdded;
}

QList<Plugin*> MainWindow::getPluginsForContext(QString context)
{
    QMap<QString,Plugin*>::iterator i;
    QList<Plugin*> r;
    for (i = plugins_.begin(); i != plugins_.end(); i++)
    {
        if (i.value()->context() == context)
        {
            r << i.value();
        }
    }
    return r;
}

void MainWindow::on_btnPlugins_clicked()
{
    if (!dlgPlugins_.isVisible())
    {
        dlgPlugins_.setVisible(true);
    }
    qInfo().noquote() << "Showing plugins window";
    dlgPlugins_.show();
    //emit ???
    dlgPlugins_.setFocus();
}

void MainWindow::on_PluginDialogReady()
{
    ui->btnPlugins->setEnabled(true);
}

QString MainWindow::humanFriendlyFileSize(qint64 sizeInBytes)
{
    if (sizeInBytes >= 1024 * 1024)
        return QString().sprintf("%.1fmb", sizeInBytes / (1024.0 * 1024));
    if (sizeInBytes >= 16 * 1024)
        return QString().sprintf("%lldkb", sizeInBytes / 1024);
    return QString().sprintf("%lld", sizeInBytes);
}
