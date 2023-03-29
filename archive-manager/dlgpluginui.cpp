#include "dlgpluginui.h"
#include "ui_dlgpluginui.h"

#include "plugin.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QComboBox>

DlgPluginUI::DlgPluginUI(Plugin *p, QWidget *parent) :
    QDialog(nullptr),
    ui(new Ui::DlgPluginUI)
{
    main_ = qobject_cast<MainWindow*>(parent);
    plugin_ = p;
    // Set up relay
    connect( plugin_, SIGNAL(outputLine(QString)), SIGNAL(logging(QString)));
    ui->setupUi(this);
    // Continue after creator has hooked up signals
    QTimer::singleShot(100, this, SLOT(delayedInit()));
}

void DlgPluginUI::delayedInit()
{
    form_ = plugin_->form();
    QString title(plugin_->id());
    if (form_.contains("Title"))
    {
        title = form_["Title"].toString();
    }
    setWindowTitle(title);
    emit logging("Started dialog for " + title);
    bool validForm = false;
    if (form_.contains("Fields") && form_["Fields"].isArray())
    {
        QJsonArray fields = form_["Fields"].toArray();
        emit logging(QString().sprintf("%d fields", fields.count()));
        for (int n = 0; n < fields.count(); n++)
        {
            // ID, PassAs, Label and Type are required
            // Index is required for PassAs==arg
            // EnvName is required for PassAs==env
            // Required, Persist and Default are optional
            QJsonObject field = fields.at(n).toObject();
            if (field.contains("ID") && field.contains("PassAs") && field.contains("Label") && field.contains("Type"))
            {
                // Valid
                validForm = true;
                if (field["Type"] == "text")
                {
                    QLineEdit *txt = new QLineEdit(ui->formLayout->widget());
                    PluginField f(field, txt, plugin_, this);
                    f.setupWidget();
                    ui->formLayout->addRow(field["Label"].toString(), txt);
                    fields_.push_back(f);
                }
                else if (field["Type"] == "dropdown")
                {
                    QComboBox *combo = new QComboBox(ui->formLayout->widget());
                    PluginField f(field, combo, plugin_, this);
                    f.setupWidget();
                    ui->formLayout->addRow(field["Label"].toString(), combo);
                    fields_.push_back(f);
                }
                else emit logging("Unknown type " + field["Type"].toString());
            }
            else
            {
                emit logging("Missing required attributes");
            }
        }
    }
    else
    {
        emit logging("No fields");
    }
}

DlgPluginUI::~DlgPluginUI()
{
    delete ui;
}

void DlgPluginUI::on_btnDone_clicked()
{
    close();
}


void DlgPluginUI::on_btnExecute_clicked()
{
    emit logging(QString().sprintf("--- begin execution (%d fields) ---", fields_.count() ));
    QList<QString> argList;
    QMap<QString,QString> envMap;
    int failureCount = 0;
    argList << plugin_->getScriptPath();
    for (int n = 0; n < fields_.count(); n++)
    {
        if (!fields_[n].saveOutput(argList, envMap))
        {
            failureCount++;
        }
    }
    if (failureCount > 0)
    {
        emit logging(QString().sprintf("Skipping execution due to %d validation failures", failureCount));
    }
    else
    {
        qDebug().noquote() << "args" << argList << "env" << envMap;
        argList.removeFirst();
        if (plugin_->runScript(argList, envMap))
        {
            emit logging("Execution successfully started");
        }
        else
        {
            emit logging("Execution FAILED");
        }
    }
}
