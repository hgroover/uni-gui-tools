#ifndef PLUGINFIELD_H
#define PLUGINFIELD_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>

/// Simple container for widget managing a single plugin UI field
class PluginField;
class Plugin;

class PluginField : public QObject
{
    Q_OBJECT
public:
    explicit PluginField(QJsonObject const& definition, QWidget *widget, Plugin *plugin, QObject *parent = nullptr);
    PluginField(PluginField const& src);

    PluginField const& operator=(PluginField const& src);

    bool setupWidget();

    QString getType() const { return type_; }
    QString getId() const { return id_; }
    QString getPassAs() const { return passAs_; }

    bool saveOutput(QList<QString>& args, QMap<QString,QString>& env);

signals:

public slots:

protected:
    QString type_;
    QString id_;
    QString passAs_; ///< arg or env
    int argIndex_; ///< required if passAs==arg
    QString envName_; ///< required if passAs==env
    bool required_;
    QString default_;
    bool persist_;
    QString tooltip_;
    QJsonArray elements_;
    QWidget *widget_;
    Plugin *plugin_;

    bool _setupText();
    bool _setupDropdown();
    QString _getText();
};

#endif // PLUGINFIELD_H
