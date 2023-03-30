#include "pluginfield.h"

#include "plugin.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>

PluginField::PluginField(QJsonObject const& definition, QWidget *widget, Plugin *plugin, QObject *parent)
    : QObject(parent),
      required_(false),
      persist_(false),
      argIndex_(-1),
      envName_(),
      tooltip_(),
      elements_()
{
    widget_ = widget;
    plugin_ = plugin;
    type_ = definition["Type"].toString();
    id_ = definition["ID"].toString();
    passAs_ = definition["PassAs"].toString();
    if (definition.contains("Index")) argIndex_ = definition["Index"].toInt();
    if (definition.contains("EnvName")) envName_ = definition["EnvName"].toString();
    if (definition.contains("Required")) required_ = definition["Required"].toBool();
    if (definition.contains("Persist")) persist_ = definition["Persist"].toBool();
    if (definition.contains("Default")) default_ = definition["Default"].toString();
    if (definition.contains("ToolTip")) tooltip_ = definition["ToolTip"].toString();
    if (definition.contains("Elements")) elements_ = definition["Elements"].toArray();
}

PluginField::PluginField(const PluginField &src) : QObject(src.parent())
{
    *this = src;
}

PluginField const& PluginField::operator=(const PluginField &src)
{
    type_ = src.type_;
    widget_ = src.widget_;
    plugin_ = src.plugin_;
    passAs_ = src.passAs_;
    id_ = src.id_;
    argIndex_ = src.argIndex_;
    envName_ = src.envName_;
    required_ = src.required_;
    persist_ = src.persist_;
    default_ = src.default_;
    tooltip_ = src.tooltip_;
    elements_ = src.elements_;
    return *this;
}

bool PluginField::setupWidget()
{
    if (type_ == "text")
    {
        return _setupText();
    }
    else if (type_ == "dropdown")
    {
        return _setupDropdown();
    }
    qWarning().noquote() << "Invalid type" << type_;
    return false;
}

bool PluginField::_setupText()
{
    QLineEdit *txt = qobject_cast<QLineEdit*>(widget_);
    if (!default_.isEmpty()) txt->setText(default_);
    if (!tooltip_.isEmpty()) txt->setToolTip(tooltip_);
    return true;
}

bool PluginField::_setupDropdown()
{
    QComboBox *combo = qobject_cast<QComboBox*>(widget_);
    if (!tooltip_.isEmpty()) combo->setToolTip(tooltip_);
    if (elements_.isEmpty())
    {
        qWarning().noquote() << "Elements empty for" << id_;
        return false;
    }
    for (int n = 0; n < elements_.count(); n++)
    {
        QJsonObject e = elements_[n].toObject();
        if (!e.contains("Value"))
        {
            qWarning().noquote() << "No Value for Elements[" << n << "] in" << id_;
            continue;
        }
        QString value(e["Value"].toString());
        QString label(value);
        if (e.contains("Label"))
        {
            label = e["Label"].toString();
        }
        qDebug().noquote() << "Adding" << label << "," << value;
        combo->addItem(label, value);
    }
    if (!default_.isEmpty())
    {
        combo->setCurrentText(default_);
    }
    return true;
}

QString PluginField::_getText()
{
    if (type_ == "text")
    {
        return qobject_cast<QLineEdit*>(widget_)->text();
    }
    if (type_ == "dropdown")
    {
        return qobject_cast<QComboBox*>(widget_)->currentData().toString();
    }
    return QString();
}

bool PluginField::saveOutput(QList<QString> &args, QMap<QString, QString> &env)
{
    QString s(_getText());
    if (required_ && s.isEmpty())
    {
        qWarning().noquote() << "Validation fails for" << id_;
        return false;
    }
    if (passAs_ == "arg")
    {
        if (argIndex_ < 0)
        {
            qWarning().noquote() << "Invalid arg index for" << id_;
            return false;
        }
        while (args.count() <= argIndex_) args.push_back("");
        args[argIndex_] = s;
    }
    else if (passAs_ == "env")
    {
        if (envName_.isEmpty())
        {
            qWarning().noquote() << "Missing EnvName for" << id_;
            return false;
        }
        env[envName_] = s;
    }
    else
    {
        qWarning().noquote() << "Invalid PassAs" << passAs_;
        return false;
    }
    return true;
}
