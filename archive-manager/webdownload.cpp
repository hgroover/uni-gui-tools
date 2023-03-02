#include "webdownload.h"
#include "ui_webdownload.h"

#include "mainwindow.h"

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
    ui->lstFiles->clearSelection();
}

void WebDownload::on_btnRefresh_clicked()
{
    QString baseUrl(ui->txtUrl->text());
    if (baseUrl.isEmpty()) return;
    rebuildList(baseUrl);
}

void WebDownload::on_btnDone_clicked()
{
    QList<QListWidgetItem*> selected = ui->lstFiles->selectedItems();
    qInfo().noquote() << selected.count() << "files selected for download";
    for (int n = 0; n < selected.count(); n++)
    {
        qInfo().noquote() << '[' << n << ']' << selected[n]->text();
        beginDownload(selected[n]->text().mid(2));
    }
    hide();
    if (parent() != nullptr)
    {
        qInfo().noquote() << "Returning focus to main window";
        ((QWidget*)parent())->setFocus();
    }
}

void WebDownload::on_UpdatedLogDir(QString logDir)
{
    downloadDir_ = logDir;
}

void WebDownload::on_UpdatedBaseUrl(QString baseUrl)
{
    ui->txtUrl->setText(baseUrl);
    if (baseUrl.isEmpty()) return;
    rebuildList(baseUrl);
}

void WebDownload::rebuildList(QString baseUrl)
{
    qInfo().noquote() << "Reading" << baseUrl;
    const QUrl url(baseUrl);

    QNetworkReply* reply = net_.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
      on_ReplyFinished(reply);
    });

}

void WebDownload::beginDownload(QString filename)
{
    QString fullUrl(ui->txtUrl->text());
    if (!fullUrl.endsWith('/')) fullUrl += '/';
    fullUrl += filename;
    QString fullSavePath(downloadDir_);
    if (!fullSavePath.endsWith('/')) fullSavePath += '/';
    fullSavePath += filename;
    qInfo().noquote() << "Downloading" << fullUrl << "to" << fullSavePath;
    const QUrl url(fullUrl);
    QNetworkReply *reply = net_.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, reply, fullSavePath]() {
        on_DownloadFinished(reply, fullSavePath);
    });
}

// Useful anywhere test value: http://ftp.gnu.org/gnu/gcal/
void WebDownload::on_ReplyFinished(QNetworkReply *reply)
{
    QString rawData = reply->readAll();
    //qInfo().noquote() << rawData;
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
                    // Use prefix L if cached locally, d if downloadable
                    aFiles.push_back(fnCandidate);
                    QChar prefix('d');
                    MainWindow *pMain = static_cast<MainWindow*>(parent());
                    if (pMain->hasLocalCopy(fnCandidate))
                    {
                        prefix = 'L';
                    }
                    ui->lstFiles->addItem(prefix + '-' + fnCandidate);
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

void WebDownload::on_DownloadFinished(QNetworkReply *reply, QString savePath)
{
    QByteArray ba(reply->readAll());
    if (ba.length() > 0)
    {
        qInfo().noquote() << "Download complete, size:" << ba.length() << "saving to:" << savePath;
        QFile d(savePath);
        if (d.open(QIODevice::WriteOnly))
        {
            d.write(ba);
            d.close();
            qInfo().noquote() << "Written";
            QFileInfo f(savePath);
            emit downloadCompleted(f.fileName());
        }
        else
        {
            qWarning().noquote() << "Failed to open for write:" << savePath;
        }
    }
    else
    {
        qInfo().noquote() << "Download empty";
    }
    reply->deleteLater();
}

void WebDownload::on_UpdatedFilespec(QString filespec)
{
    fileFilter_ = filespec;
}
