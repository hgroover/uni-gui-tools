#ifndef DLGCONFIG_H
#define DLGCONFIG_H

#include <QDialog>

namespace Ui {
class dlgConfig;
}

class dlgConfig : public QDialog
{
    Q_OBJECT

public:
    explicit dlgConfig(QWidget *parent = nullptr);
    ~dlgConfig();

public slots:
    void loadValues();
    void saveValues();

signals:
    void updatedLogDir(QString logDir);
    void updatedLogFilespec(QString logFilespec);
    void updatedGitDir(QString gitDir);
    void updatedExtractionDir(QString extractionDir);
    void updatedExtractScript(QString extractScript);
    void updatedBaseUrl(QString baseUrl);
    // Value changes in View tab
    void updatedViewExternalViewerTrigger(int fileSize);
    void updatedViewExternalViewer(QString appPath);

private slots:
    void on_buttonBox_accepted();

    void on_lblLogdir_linkActivated(const QString &link);

    void on_lblGitDir_linkActivated(const QString &link);

    void on_lblExtractDir_linkActivated(const QString &link);

    void on_lblExtractScript_linkActivated(const QString &link);

    void on_lblViewExternalViewer_linkActivated(const QString &link);

private:
    Ui::dlgConfig *ui;
};

#endif // DLGCONFIG_H
