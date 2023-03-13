#include "basicfileviewer.h"
#include "ui_basicfileviewer.h"

#include <QUrl>
#include <QDir>
#include <QJsonDocument>

BasicFileViewer::BasicFileViewer(QWidget *parent, QString logDir, QString extractedFile) :
    QDialog(parent),
    ui(new Ui::BasicFileViewer),
    detected_(FT_UNKNOWN)
{
    ui->setupUi(this);
    ui->lblArchiveInfo->setText("Archive: " + logDir);
    ui->lblFileInfo->setText("File: " + extractedFile);
    QDir d(logDir);
    logDir_ = logDir;
    extractedFile_ = extractedFile;
    extractedFilePath_ = d.absoluteFilePath(extractedFile);
    ui->txtContents->setVisible(false);
    ui->txtContentsPlain->setVisible(true);
    // This doesn't do anything
    //ui->txtContents->setAcceptRichText(false);
    //ui->txtContents->setSource(QUrl::fromLocalFile(extractedFilePath_));
    //QTextDocument *doc = new QTextDocument(this);
    //doc->addResource();
    //ui->txtContentsPlain->setDocument(doc);
    QFile f(extractedFilePath_);
    if (f.open(QIODevice::ReadOnly))
    {
        QString sContents(f.readAll());
        f.close();
        // Detect file type
        QString sBegin(sContents.left(256));
        if (sBegin.contains("<html>"))
        {
            detected_ = FT_HTML;
        }
        else if (sBegin.trimmed().startsWith('{'))
        {
            detected_ = FT_JSON;
        }
        else
        {
            // There's little chance that we would have a binary file without some unprintable characters in the first 256 bytes
            // FIXME need a better binary test!!
            if (sBegin.toLatin1() == sBegin.toLocal8Bit())
            {
                detected_ = FT_PLAINTEXT;
            }
            else
            {
                detected_ = FT_BINARY;
            }
        }
        switch (detected_)
        {
        case FT_HTML:
            ui->txtContents->setVisible(true);
            ui->txtContentsPlain->setVisible(false);
            ui->txtContents->setText(sContents);
            break;
        case FT_JSON:
            {
            QJsonDocument doc = QJsonDocument::fromJson(sContents.toUtf8());
            QString formattedJsonString = doc.toJson(QJsonDocument::Indented);
            ui->txtContentsPlain->setPlainText(formattedJsonString);
            }
            break;
        case FT_PLAINTEXT:
        case FT_BINARY:
        case FT_UNKNOWN:
            ui->txtContentsPlain->setPlainText(sContents);
            break;
        }
    }
    else
    {
        ui->txtContentsPlain->setPlainText("Failed to open " + extractedFilePath_);
    }
}

BasicFileViewer::~BasicFileViewer()
{
    delete ui;
}

void BasicFileViewer::on_btnDone_clicked()
{
    close();
    deleteLater();
}
