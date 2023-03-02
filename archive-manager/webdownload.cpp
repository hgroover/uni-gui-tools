#include "webdownload.h"
#include "ui_webdownload.h"

#include <QtDebug>
#include <QRegExp>

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

// Useful anywhere test value: http://ftp.gnu.org/gnu/gcal/
void WebDownload::on_ReplyFinished(QNetworkReply *reply)
{
    QString rawData = reply->readAll();
    qInfo().noquote() << rawData;
    QStringList a = rawData.split('\n', QString::SkipEmptyParts);
    // Extract filenames from href="" tags
    QStringList aFiles;
    QRegExp reHref("href=\"([^\"]*)\"");
    ui->lstFiles->clear();
    int rejectCount = 0;
    for (int n = 0; n < a.size(); n++)
    {
        if (reHref.indexIn(a[n]) >= 0)
        {
            QString fnCandidate(reHref.cap(1));
            if (fnCandidate.isEmpty() || fnCandidate[0] == '.')
            {
                qDebug().noquote() << "Ignoring" << fnCandidate;
            }
            else
            {
                QRegExp reFilespec(fileFilter_);
                reFilespec.setPatternSyntax(QRegExp::Wildcard);
                if (reFilespec.exactMatch(fnCandidate))
                {
                    aFiles.push_back(fnCandidate);
                    ui->lstFiles->addItem(fnCandidate);
                }
                else
                {
                    qInfo().noquote() << "Does not match:" << fnCandidate;
                    rejectCount++;
                }
            }
        }
    }
    ui->lblStatus->setText(QString().sprintf("Received %d bytes, %d lines, %d files (%d links rejected)",
                                             rawData.length(), a.size(), aFiles.size(), rejectCount));
    reply->deleteLater();
}

void WebDownload::on_UpdatedFilespec(QString filespec)
{
    fileFilter_ = filespec;
}
