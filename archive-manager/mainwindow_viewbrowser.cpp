#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFileInfo>

void MainWindow::populateViewBrowser(bool extracted)
{
    QString html;
    if (extracted)
    {
        // If there's an insight script, run it otherwise default action is here
        html = "<h5>" + curLogExtractDir_ + "</h5>";
        html += "<table padding=\"1\"><thead><tr><th align=\"left\">File</th><th>Size</th><th align=\"left\">Details</th><</tr></thead>\n";
        html += "<tbody>\n";
        QString prefix;
        int currentRecursion = 0;
        int maxRecursion = 1;
        QDir d(curLogExtractDir_);
        QFileInfoList a = d.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);
        for (int n = 0; n < a.length(); n++)
        {
            html += viewBrowserRow(a[n], currentRecursion, maxRecursion, prefix);
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

QString MainWindow::viewBrowserRow(QFileInfo& fi, int& recursionLevel, int maxRecursion, QString prefix)
{
    QString html;
    const int shortContentSize = 80;

    bool showContents = false;
    bool showViewLink = fi.isFile();
    bool shouldRecurse = false;
    QString fileContents("&nbsp;&nbsp;&nbsp;");
    if (showViewLink && fi.size() <= shortContentSize)
    {
        // If a single line, display it
        QFile f(fi.filePath());
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
                fileContents += QString().sprintf("(%lld lines)", fi.size());
                //qInfo().noquote() << "Not showing" << a[n].size() << "byte file" << a[n].fileName() << ", lines" << a2.size();
            }
        }
    }
    html += "<tr>";
        html += "<td>";
            for (int n = 0; n < recursionLevel; n++) html += "&nbsp;|&nbsp;&nbsp;";
            if (showViewLink)
            {
                html += "<a href=\"view:";
                html += prefix;
                html += fi.fileName();
                html += "\">";
            }
            html += fi.fileName();
            if (showViewLink)
            {
                html += "</a>";
            }
        html += "</td>";
        html += "<td align=\"right\">";
            if (fi.isFile())
            {
                html += QString().sprintf("%lld", fi.size());
            }
            else
            {
                html += "&lt;DIR&gt;";
                shouldRecurse = (recursionLevel < maxRecursion);
            }
        html += "</td>";
        html += "<td align=\"left\">";
            html += fileContents;
        html += "</td>";
    html += "</tr>\n";

    if (shouldRecurse)
    {
        recursionLevel++;
        prefix = fi.fileName() + '/';
        QDir d(fi.filePath());
        QFileInfoList a = d.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);
        for (int n = 0; n < a.length(); n++)
        {
            html += viewBrowserRow(a[n], recursionLevel, maxRecursion, prefix);
        }
        recursionLevel--;
    }

    return html;
}
