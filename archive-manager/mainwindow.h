#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPair>

#include "webdownload.h"
#include "loglistmodel.h"
#include "plugin.h"
#include "dlgplugins.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QString tarballBasename(QString tarballFilename);
    static QString bashify(QString windowsPath);
    void setLogVerboseLevel(int level) { g_verbose = level; }

    // Provide a variant shell command line that will work on Windows+gitbash or Linux or OSX
    QString shellCmdLine(QString shellPath, QStringList shellArgs = QStringList());

    // Get list of plugins for specific context
    QList<Plugin*> getPluginsForContext(QString context);

    static int g_verbose;

signals:
    void updatedBaseUrl(QString baseUrl);
    void updatedLogFilespec(QString filespec);
    void updatedLogDir(QString logDir);
    void pluginListReady();

public slots:
    bool hasLocalCopy(QString logFile);
    void selectLogSort(QString keyId);
    void addPlugin(Plugin *p);
    void removePlugin(Plugin *p);

protected slots:
    void on_LogDirChanged(QString logDir);
    void on_LogFilespecChanged(QString logFilespec);
    void on_GitDirChanged(QString gitDir);
    void on_ExtractionDirChanged(QString extractionDir);
    void on_ExtractScriptChanged(QString extractScript);
    void on_BaseUrlChanged(QString baseUrl);
    void on_ViewExternalSizeTrigger(int fileSize);
    void on_ViewExternalViewer(QString viewerApp);
    void on_Refresh();
    void populateViewBrowser(bool extracted); ///< In mainwindow_viewbrowser.cpp
    QString viewBrowserRow(QFileInfo &fi, int& recursionLevel, int maxRecursion, QString prefix);
    void on_DownloadCompleted(QString filename);
    void on_AssertFirstSelection();
    void on_InitialLoad();
    void on_PluginDialogReady();
    int loadPlugins();
    void testAnimator();

    // in mainwindow_extract.cpp
    void on_ExtractionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_ExtractionReadyRead();
    void on_ExtractionError(QProcess::ProcessError error);
    void on_PostFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_PostError(QProcess::ProcessError error);
    void refreshCurrent();

private slots:
    // Where we save state/position. Ideally use installEventFilter() and get resize and change state events
    // so we can handle user closing window via X in title bar
    void on_btnQuit_clicked();

    void on_btnConfigure_clicked();

    void on_btnRefresh_clicked();

    //void on_lstLogs_currentRowChanged(int currentRow);

    void on_btnLogExtract_clicked();

    void on_btnView_clicked();

    void on_btnClear_clicked();

    void on_txtItemDetails_anchorClicked(const QUrl &arg1);

    void on_btnDownload_clicked();

    void on_lblGlobalOptions_linkActivated(const QString &link);

    void on_lblGlobalOptions_linkHovered(const QString &link);

    void on_lblSortOptions_linkActivated(const QString &link);

    void on_lblSortOptions_linkHovered(const QString &link);

    void on_lstLogsModel_clicked(const QModelIndex &index);

    void on_btnPlugins_clicked();

protected:
    // In mainwindow_extract.cpp
    QProcess *prepareRunner(bool post);


private:
    Ui::MainWindow *ui;
    WebDownload web_;
    DlgPlugins dlgPlugins_;

    // Set in config dialog
    QString logDir_;
    QString logFilespec_;
    QString gitDir_;
    QString gitBinDir_;
    QString gitShellPath_;
    QString extractDir_;
    QString extractScript_;
    QString baseUrl_;
    int viewExternalViewTrigger_;
    QString viewExternalViewerApp_;

    bool extractionValid_;
    bool bashValid_;
    bool extractScriptValid_;
    int logListDirty_;
    static const QList<QList<QString>> a_sortLinks;

    // Current item row values set by on_lstLogs_currentRowChanged
    int curSelectedRow_;
    QString curLogFilename_;
    QString curLogExtractDir_;
    QString curLogTarballPath_;

    LogListModel *logList_;
    QMap<QString,Plugin*> plugins_;

};

#endif // MAINWINDOW_H
