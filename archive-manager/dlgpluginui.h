#ifndef DLGPLUGINUI_H
#define DLGPLUGINUI_H

#include <QDialog>
#include <QJsonDocument>
#include <QList>

#include "mainwindow.h"
#include "plugin.h"

namespace Ui {
class DlgPluginUI;
}

class DlgPluginUI : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPluginUI(Plugin *p, QWidget *parent = 0);
    ~DlgPluginUI();

signals:
    void logging(QString text);

private slots:
    void on_btnDone_clicked();
    void delayedInit();


    void on_btnExecute_clicked();

private:
    Ui::DlgPluginUI *ui;
    MainWindow *main_;
    Plugin *plugin_;
    QJsonObject form_;
    QList<PluginField> fields_;

};

#endif // DLGPLUGINUI_H
