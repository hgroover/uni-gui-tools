#include "dlgplugins.h"
#include "ui_dlgplugins.h"

#include "archive-manager-globals.h"
#include "mainwindow.h"
#include "dlgpluginui.h"

#include <QDebug>
#include <QPushButton>
#include <QLabel>

DlgPlugins::DlgPlugins(QWidget *parent) :
    QDialog(nullptr),
    ui(new Ui::DlgPlugins)
{
    main_ = qobject_cast<MainWindow*>(parent);
    ui->setupUi(this);
    setWindowTitle("Plugins");
    MYQSETTINGS(settings);
    restoreGeometry(settings.value(cfg_pw_geometry).toByteArray());
    //restoreState(settings.value(cfg_pw_state).toByteArray());
}

DlgPlugins::~DlgPlugins()
{
    delete ui;
}

void DlgPlugins::on_btnDone_clicked()
{
    MYQSETTINGS(settings);
    settings.setValue(cfg_pw_geometry, saveGeometry());
    //settings.setValue(cfg_pw_state, saveState());
    hide();
    if (main_ != nullptr)
    {
        qInfo().noquote() << "Returning focus to main window";
        main_->setFocus();
    }

}

void DlgPlugins::on_pluginListReady()
{
    if (main_ != nullptr)
    {
        uiPlugins_ = main_->getPluginsForContext("gui");
        if (uiPlugins_.length() > 0)
        {
            emit pluginDialogReady();
            on_log(QString().sprintf("Loaded %d ui plugins", uiPlugins_.length()));
            QTimer::singleShot(0, this, SLOT(on_rebuildUI()));
        }
        else
        {
            qWarning().noquote() << "No ui plugins";
        }
    }
}

void DlgPlugins::on_log(QString text)
{
    qDebug().noquote() << "log" << text;
    ui->txtOutput->append(text);
}

void DlgPlugins::on_rebuildUI()
{
    int removalCount = 0;
    while (ui->layoutFormPlugins->rowCount() > 0)
    {
        ui->layoutFormPlugins->removeRow(0);
        removalCount++;
    }
    qDebug() << removalCount << "entries removed";
    for (int n = 0; n < uiPlugins_.length(); n++)
    {
        addPluginUI(uiPlugins_[n]);
    }
    qDebug() << uiPlugins_.count() << "entries added";
}

void DlgPlugins::addPluginUI(Plugin *p)
{
    QPushButton *field = new QPushButton(QString("Launch"), ui->layoutFormPlugins->widget());
    widgetToPlugin_[(QWidget*)field] = p;
    connect(field, SIGNAL(clicked(bool)), this, SLOT(on_btn_clicked(bool)));
    field->setMaximumWidth(120);
    field->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    QLabel *label = new QLabel(p->id(), ui->layoutFormPlugins->widget());
    label->setMinimumWidth(240);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    ui->layoutFormPlugins->addRow(label, field);
}

void DlgPlugins::on_btn_clicked(bool checked)
{
    Plugin *p = nullptr;
    QWidget *senderWidget = qobject_cast<QWidget*>(sender());
    if (widgetToPlugin_.contains(senderWidget))
    {
        p = widgetToPlugin_[senderWidget];
        qDebug().noquote() << "got" << p->id() << "checked" << checked;
        DlgPluginUI *dlg = new DlgPluginUI(p, main_);
        connect(dlg, SIGNAL(logging(QString)), this, SLOT(on_log(QString)));
        dlg->show();
    }
    else
    {
        qDebug() << "Failed to get plugin";
    }
}
