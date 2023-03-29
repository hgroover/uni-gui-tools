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
                    ui->formLayout->addRow(field["Label"].toString(), txt);
                }
                else if (field["Type"] == "dropdown")
                {
                    QComboBox *combo = new QComboBox(ui->formLayout->widget());
                    ui->formLayout->addRow(field["Label"].toString(), combo);
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
    emit logging("--- begin execution ---");
}
