#include "basicfileviewer.h"
#include "ui_basicfileviewer.h"

#include <QUrl>
#include <QDir>

BasicFileViewer::BasicFileViewer(QWidget *parent, QString logDir, QString extractedFile) :
    QDialog(parent),
    ui(new Ui::BasicFileViewer)
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
        ui->txtContentsPlain->setPlainText(f.readAll());
        f.close();
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
