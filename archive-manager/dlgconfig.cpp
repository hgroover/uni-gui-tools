#include "dlgconfig.h"
#include "ui_dlgconfig.h"
#include "archive-manager-globals.h"

#include <QSettings>
#include <QFileDialog>

dlgConfig::dlgConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgConfig)
{
    ui->setupUi(this);
}

dlgConfig::~dlgConfig()
{
    delete ui;
}

void dlgConfig::on_buttonBox_accepted()
{
    // Save changed values
    MYQSETTINGS(settings);
    settings.setValue("logDir", ui->txtLogDir->text());
    settings.setValue("logFilespec", ui->txtLogFilespec->text());
    settings.setValue("gitDir", ui->txtGitDir->text());
    settings.setValue("extractionDir", ui->txtExtractionDir->text());
    settings.setValue("extractScript", ui->txtExtractScript->text());
    settings.setValue("baseUrl", ui->txtBaseUrl->text());
    // Notify
    emit updatedLogDir(ui->txtLogDir->text());
    emit updatedLogFilespec(ui->txtLogFilespec->text());
    emit updatedGitDir(ui->txtGitDir->text());
    emit updatedExtractionDir(ui->txtExtractionDir->text());
    emit updatedExtractScript(ui->txtExtractScript->text());
    emit updatedBaseUrl(ui->txtBaseUrl->text());
}

void dlgConfig::on_lblLogdir_linkActivated(const QString &link)
{
    if (link == "edit")
    {
        // Use picker
        ui->txtLogDir->setText(QFileDialog::getExistingDirectory(this, "Select directory containing log archives"));
    }
    else // if (link == "reset")
    {
        // Revert
        MYQSETTINGS(settings);
        ui->txtLogDir->setText(settings.value("logDir").toString());
    }
}

void dlgConfig::setGitDir(QString gitDir)
{
    ui->txtGitDir->setText(gitDir);
}

void dlgConfig::setLogDir(QString logDir)
{
    ui->txtLogDir->setText(logDir);
}

void dlgConfig::setLogFilespec(QString logFilespec)
{
    ui->txtLogFilespec->setText(logFilespec);
}

void dlgConfig::setExtractionDir(QString extractionDir)
{
    ui->txtExtractionDir->setText(extractionDir);
}

void dlgConfig::setExtractScript(QString extractScript)
{
    ui->txtExtractScript->setText(extractScript);
}

void dlgConfig::setBaseUrl(QString baseUrl)
{
    ui->txtBaseUrl->setText(baseUrl);
}

void dlgConfig::on_lblGitDir_linkActivated(const QString &link)
{
    if (link == "change")
    {
        ui->txtGitDir->setText(QFileDialog::getExistingDirectory(this, "Select directory where git bash was installed"));
    }
    else // if (link == "reset")
    {
        // Revert
        MYQSETTINGS(settings);
        ui->txtGitDir->setText(settings.value("gitDir").toString());
    }
}

void dlgConfig::on_lblExtractDir_linkActivated(const QString &link)
{
    if (link == "change")
    {
        ui->txtExtractionDir->setText(QFileDialog::getExistingDirectory(this, "Select directory to extract archived logs"));
    }
    else // if (link == "reset")
    {
        MYQSETTINGS(settings);
        ui->txtExtractionDir->setText(settings.value("extractionDir").toString());
    }
}

void dlgConfig::on_lblExtractScript_linkActivated(const QString &link)
{
    if (link == "change")
    {
        ui->txtExtractScript->setText(QFileDialog::getOpenFileName(this, "Select bash script to extract archive"));
    }
    else // if (link == "reset")
    {
        MYQSETTINGS(settings);
        ui->txtExtractScript->setText(settings.value("extractScript").toString());
    }
}
