#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QJsonDocument>

// See PLUGINS.md

class Plugin : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        DS_INIT=0,
        DS_GETDEFS,
        DS_BADDEFS,
        DS_INVALID,
        DS_READY
    } DataState_t;

    explicit Plugin(QObject *parent = nullptr);
    void init(QString scriptPath);

    const QString &id() const { return id_; }
    const QString &context() const { return context_; }
    bool isValid() const { return !(id_.isEmpty() || context_.isEmpty()); }
    bool isVisible() const { return true; }
    QByteArray runScript(QStringList args, bool& success);
    QJsonObject form();

signals:

public slots:

protected:
    QString id_;
    QString context_;
    QString scriptPath_;
    QJsonDocument defs_;
    DataState_t curState_;
};

#endif // PLUGIN_H
