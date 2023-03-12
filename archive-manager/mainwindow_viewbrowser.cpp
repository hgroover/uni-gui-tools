#include "mainwindow.h"

#include <QDebug>
#include <QString>
#include <QFileInfo>

void MainWindow::populateViewBrowser(bool extracted)
{
    QString html;
    const int shortContentSize = 80;
    if (extracted)
    {
        // If there's an insight script, run it otherwise default action is here
        html = "<h5>" + curLogExtractDir_ + "</h5>";
        html += "<table padding=\"1\"><thead><tr><th align=\"left\">File</th><th>Size</th><th align=\"left\">Details</th><</tr></thead>\n";
        html += "<tbody>\n";
        QDir d(curLogExtractDir_);
        QFileInfoList a = d.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);
        for (int n = 0; n < a.length(); n++)
        {
            bool showContents = false;
            bool showViewLink = a[n].isFile();
            QString fileContents("&nbsp;&nbsp;&nbsp;");
            if (showViewLink && a[n].size() <= shortContentSize)
            {
                // If a single line, display it
                QFile f(a[n].filePath());
                if (f.open(QIODevice::ReadOnly))
                {
                    QList<QByteArray> a2 = f.readAll().split('\n');
                    if (a2.size()<=2)
                    {
                        showContents = true;
                        fileContents += a2[0];
                        showViewLink = false;
                    }
                    else
                    {
                        fileContents += QString().sprintf("(%lld lines)", a[n].size());
                        //qInfo().noquote() << "Not showing" << a[n].size() << "byte file" << a[n].fileName() << ", lines" << a2.size();
                    }
                }
            }
            html += "<tr>";
                html += "<td>";
                    if (showViewLink)
                    {
                        html += "<a href=\"view:";
                        html += a[n].fileName();
                        html += "\">";
                    }
                    html += a[n].fileName();
                    if (showViewLink)
                    {
                        html += "</a>";
                    }
                html += "</td>";
                html += "<td align=\"right\">";
                    if (a[n].isFile())
                    {
                        html += QString().sprintf("%lld", a[n].size());
                    }
                    else
                    {
                        html += "&lt;DIR&gt;";
                    }
                html += "</td>";
                html += "<td align=\"left\">";
                    html += fileContents;
                html += "</td>";
            html += "</tr>\n";
        }
        html += "</tbody>\n";
        html += "</table>\n";
    }
    else
    {
        html = "<h4>No contents</h4><p><a href=\"extract:" + curLogFilename_ + "\">click here</a> or extract button</p>";
    }
    ui->txtItemDetails->setText(html);
}

