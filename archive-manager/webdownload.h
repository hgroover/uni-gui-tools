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

public slots:
    void on_UpdatedBaseUrl(QString baseUrl);
    void on_ReplyFinished(QNetworkReply *reply);

private slots:
    void on_btnUnselect_clicked();

    void on_btnRefresh_clicked();

    void on_btnDone_clicked();

private:
    Ui::WebDownload *ui;
    QNetworkAccessManager net_;
};

#endif // WEBDOWNLOAD_H
