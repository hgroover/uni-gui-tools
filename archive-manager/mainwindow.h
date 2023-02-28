#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

protected slots:
    void on_LogDirChanged(QString logDir);
    void on_LogFilespecChanged(QString logFilespec);
    void on_GitDirChanged(QString gitDir);
    void on_ExtractionDirChanged(QString extractionDir);
    void on_ExtractScriptChanged(QString extractScript);
    void on_Refresh();

private slots:
    void on_pushButton_clicked();

    void on_btnQuit_clicked();

    void on_btnConfigure_clicked();

    void on_btnRefresh_clicked();

    void on_lstLogs_currentRowChanged(int currentRow);

    void on_btnLogExtract_clicked();

    void on_btnView_clicked();

    void on_btnClear_clicked();

private:
    Ui::MainWindow *ui;
    QString logDir_;
    QString logFilespec_;
    QString gitDir_;
    QString gitBinDir_;
    QString gitShellPath_;
    QString extractDir_;
    QString extractScript_;
    bool extractionValid_;
    bool bashValid_;
    bool extractScriptValid_;
    int logListDirty_;

    // Current item row values set by on_lstLogs_currentRowChanged
    int curSelectedRow_;
    QString curLogFilename_;
    QString curLogExtractDir_;
    QString curLogTarballPath_;
};

#endif // MAINWINDOW_H
