#include "webdownload.h"
#include "ui_webdownload.h"

#include <QtDebug>

WebDownload::WebDownload(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WebDownload),
    net_()
{
    ui->setupUi(this);
}

WebDownload::~WebDownload()
{
    delete ui;
}

void WebDownload::on_btnUnselect_clicked()
{

}

void WebDownload::on_btnRefresh_clicked()
{

}

void WebDownload::on_btnDone_clicked()
{
    hide();
    if (parent() != nullptr)
    {
        qInfo().noquote() << "Returning focus to main window";
        ((QWidget*)parent())->setFocus();
    }
}

void WebDownload::on_UpdatedBaseUrl(QString baseUrl)
{
    ui->txtUrl->setText(baseUrl);
    if (baseUrl.isEmpty()) return;
    qInfo().noquote() << "Reading" << baseUrl;
    const QUrl url(baseUrl);

    QNetworkReply* reply = net_.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
      on_ReplyFinished(reply);
    });

}

void WebDownload::on_ReplyFinished(QNetworkReply *reply)
{
    qInfo().noquote() << reply->readAll();
    reply->deleteLater();
}
