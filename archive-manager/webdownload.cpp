#include "webdownload.h"
#include "ui_webdownload.h"
#include "archive-manager-globals.h"

#include "mainwindow.h"

#include <QtDebug>
#include <QRegExp>

WebDownload::WebDownload(QWidget *parent) :
    QMainWindow(nullptr),
    ui(new Ui::WebDownload),
    net_(),
    prefixLen_(6) // "[NEW] " or "[lcl] "
{
    main_ = qobject_cast<MainWindow*>(parent);
    ui->setupUi(this);
    MYQSETTINGS(settings);
    restoreGeometry(settings.value(cfg_dw_geometry).toByteArray());
    restoreState(settings.value(cfg_dw_state).toByteArray());
    ui->lstFiles->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->lstFiles->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
        // Remove optional [date] size
        QString sFile(selected[n]->text().mid(prefixLen_));
        if (sFile.contains(" ["))
        {
            sFile = sFile.left(sFile.indexOf(" ["));
        }
        beginDownload(sFile);
    }
    MYQSETTINGS(settings);
    settings.setValue(cfg_dw_geometry, saveGeometry());
    settings.setValue(cfg_dw_state, saveState());
    hide();
    if (main_ != nullptr)
    {
        qInfo().noquote() << "Returning focus to main window";
        main_->setFocus();
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

    // Get links
    QList<Plugin*> pluginList = main_->getPluginsForContext("download");
    qInfo().noquote() << "Download plugin count" << pluginList.size();
    QString htmlContent;
    for (int n = 0; n < pluginList.size(); n++)
    {
        bool ok;
        QByteArray html = pluginList[n]->runScript(QStringList(), ok);
        if (ok)
        {
            htmlContent += html;
            qInfo().noquote() << "Got" << html.length() << "bytes";
        }
        else
        {
            qWarning().noquote() << "Failed" << pluginList[n]->id();
        }
    }
    ui->lblLinks->setText(htmlContent);
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
    QStringList a = rawData.split('\n', QString::SkipEmptyParts);
    if (MainWindow::g_verbose > 0)
    {
        qDebug().noquote() << "Raw data -" << rawData.length() << "byte," << a.size() << "lines:";
        qDebug().noquote() << rawData;
        qDebug().noquote() << "-----------------";
        // Typical Apache, with whitespace line breaks added:
        // <tr>
        //     <td valign="top"><img src="/icons/tar.gif" alt="[   ]"></td>
        //     <td><a href="foo.tar">foo.tar</a></td>
        //     <td align="right">2023-04-18 09:17  </td>
        //     <td align="right">3.1M</td>
        //     <td>&nbsp;</td>
        // </tr>
        // Artifactory, whitespace converted to indented line breaks
        // <a href="logs_UNICORNDB_48A2E6B396BF_uncrn_48A2E6B396BF_1.727.2_PersonDetectAnimationStaysOn_0418_02_04_EDT.tar">logs_UNICORNDB_48A2E6B396BF_uncrn_48A2E6B396BF_1.727.2_PersonDetectAnimationStaysOn_0418_02_04_EDT.tar</a>
        //      18-Apr-2023 13:58
        //      340.00 KB

    }
    // Extract filenames from href="" tags
    QStringList aFiles;
    QRegExp reHref("href=\"([^\"]*)\"");
    // Get table columns 3-5
    QRegExp reTblColumns("<td[^>]*>([^<]*)\\s*</td>[^<]*<td[^>]*>(.*)\\s*</td>[^<]*<td[^>]*>([^<]*)\\s*</td>");
    QRegExp reArtifactory("<a[^>]*>[^<]*</a>\\s+(\\S+\\s+\\S+)\\s+(\\S+\\s*\\S*)");
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
                    QString prefix("[NEW] ");
                    if (main_->hasLocalCopy(fnCandidate))
                    {
                        prefix = "[lcl] ";
                        if (MainWindow::g_verbose > 0)
                        {
                            qDebug().noquote() << "Local" << fnCandidate << "a[n]" << a[n];
                        }
                    }
                    else if (MainWindow::g_verbose > 0)
                    {
                        qDebug().noquote() << "Non-local" << fnCandidate << "a[n]" << a[n];
                    }
                    QString s(prefix + fnCandidate);
                    // Try to get date (column 1) and size (column 2)
                    if (reTblColumns.indexIn(a[n]) != -1)
                    {
                        if (reTblColumns.captureCount() >= 2)
                        {
                            s += ' ';
                            s += '[';
                            s += reTblColumns.cap(1).trimmed();
                            s += ']';
                            s += ' ';
                            s += reTblColumns.cap(2).trimmed();
                        }
                        else
                        {
                            qDebug().noquote() << "capcount" << reTblColumns.captureCount() << reTblColumns.cap(0);
                        }
                    }
                    else if (reArtifactory.indexIn(a[n]) != -1)
                    {
                        if (reArtifactory.captureCount() >= 2)
                        {
                            s += ' ';
                            s += '[';
                            s += reArtifactory.cap(1).trimmed();
                            s += ']';
                            s += ' ';
                            s += reArtifactory.cap(2).trimmed();
                        }
                        else
                        {
                            qDebug().noquote() << "capcount" << reArtifactory.captureCount() << reArtifactory.cap(0);
                        }
                    }
                    else
                    {
                        qDebug().noquote() << "no Apache or Artifactory match:" << a[n];
                    }
                    if (MainWindow::g_verbose > 0)
                    {
                        qDebug().noquote() << "Adding to list:" << s;
                    }
                    // Turned off isWrapping which caused the second column
                    ui->lstFiles->addItem(s);
                }
                else
                {
                    if (MainWindow::g_verbose > 1)
                    {
                        qInfo().noquote() << "Does not match:" << fnCandidate;
                    }
                    rejectCount++;
                }
            }
            fnCandidate.clear();
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

void WebDownload::on_lblLinks_linkActivated(const QString &link)
{
    qInfo().noquote() << "Navigating to" << link;
    on_UpdatedBaseUrl(link);
}
