#include "dlgconfig.h"
#include "ui_dlgconfig.h"
#include "archive-manager-globals.h"

#include <QSettings>
#include <QFileDialog>
#include <QDebug>

dlgConfig::dlgConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgConfig)
{
    ui->setupUi(this);
    MYQSETTINGS(settings);
    restoreGeometry(settings.value(cfg_cw_geometry).toByteArray());
    //restoreState(settings.value(cfg_cw_state).toByteArray());
}

dlgConfig::~dlgConfig()
{
    delete ui;
}

void dlgConfig::on_buttonBox_accepted()
{
    saveValues();
    // Notify
    emit updatedLogDir(ui->txtLogDir->text());
    emit updatedLogFilespec(ui->txtLogFilespec->text());
    emit updatedGitDir(ui->txtGitDir->text());
    emit updatedExtractionDir(ui->txtExtractionDir->text());
    emit updatedExtractScript(ui->txtExtractScript->text());
    emit updatedBaseUrl(ui->txtBaseUrl->text());
    emit updatedViewExternalViewer(ui->txtViewExternalViewer->text());
    emit updatedViewExternalViewerTrigger(ui->txtViewSizeTrigger->text().toInt());
}

void dlgConfig::saveValues()
{
    // Save changed values
    MYQSETTINGS(settings);
    settings.setValue(cfg_cw_geometry, saveGeometry());
    settings.setValue(cfg_logDir, ui->txtLogDir->text());
    settings.setValue(cfg_logFilespec, ui->txtLogFilespec->text());
    settings.setValue(cfg_gitDir, ui->txtGitDir->text());
    settings.setValue(cfg_extractionDir, ui->txtExtractionDir->text());
    settings.setValue(cfg_extractionScript, ui->txtExtractScript->text());
    settings.setValue(cfg_baseUrl, ui->txtBaseUrl->text());
    settings.setValue(cfg_viewSizeTrigger, ui->txtViewSizeTrigger->text());
    settings.setValue(cfg_viewExternalViewer, ui->txtViewExternalViewer->text());
}

void dlgConfig::loadValues()
{
    MYQSETTINGS(settings);
    ui->txtLogDir->setText(settings.value(cfg_logDir).toString());
    ui->txtLogFilespec->setText(settings.value(cfg_logFilespec).toString());
    ui->txtGitDir->setText(settings.value(cfg_gitDir).toString());
    ui->txtExtractionDir->setText(settings.value(cfg_extractionDir).toString());
    ui->txtExtractScript->setText(settings.value(cfg_extractionScript).toString());
    ui->txtBaseUrl->setText(settings.value(cfg_baseUrl).toString());
    ui->txtViewSizeTrigger->setText(settings.value(cfg_viewSizeTrigger).toString());
    ui->txtViewExternalViewer->setText(settings.value(cfg_viewExternalViewer).toString());
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
        ui->txtLogDir->setText(settings.value(cfg_logDir).toString());
    }
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
        ui->txtGitDir->setText(settings.value(cfg_gitDir).toString());
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
        ui->txtExtractionDir->setText(settings.value(cfg_extractionDir).toString());
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
        ui->txtExtractScript->setText(settings.value(cfg_extractionScript).toString());
    }
}

void dlgConfig::on_lblViewExternalViewer_linkActivated(const QString &link)
{
    // Use common picker
    if (link == "select")
    {
        ui->txtViewExternalViewer->setText(QFileDialog::getOpenFileName(this, "Select external program to view large files"));
    }
    else if (link == "clear")
    {
        ui->txtViewExternalViewer->setText("");
    }
    else
    {
        qWarning().noquote() << "Unexpected link" << link;
    }
}
