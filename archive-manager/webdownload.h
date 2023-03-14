#ifndef WEBDOWNLOAD_H
#define WEBDOWNLOAD_H

#include <QMainWindow>
#include <QtNetwork>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Ui {
class WebDownload;
}

class WebDownload : public QMainWindow
{
    Q_OBJECT

public:
    explicit WebDownload(QWidget *parent = nullptr);
    ~WebDownload();

signals:
    void downloadCompleted(QString logFilename);

public slots:
    void on_UpdatedBaseUrl(QString baseUrl);
    void on_UpdatedLogDir(QString logDir);
    void on_ReplyFinished(QNetworkReply *reply);
    void on_UpdatedFilespec(QString filespec);
    void beginDownload(QString filename);
    void on_DownloadFinished(QNetworkReply *reply, QString savePath);

protected slots:
    void rebuildList(QString baseUrl);

private slots:
    void on_btnUnselect_clicked();

    void on_btnRefresh_clicked();

    void on_btnDone_clicked();

private:
    Ui::WebDownload *ui;
    QNetworkAccessManager net_;
    QString fileFilter_;
    QString downloadDir_;
    int prefixLen_;

};

#endif // WEBDOWNLOAD_H
