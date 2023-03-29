#ifndef DLGPLUGINS_H
#define DLGPLUGINS_H

#include <QDialog>
#include <QList>
#include <QMap>

#include "plugin.h"
#include "pluginfield.h"

class MainWindow;

namespace Ui {
class DlgPlugins;
}

class DlgPlugins : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPlugins(QWidget *parent = 0);
    ~DlgPlugins();

signals:
    void pluginDialogReady();

public slots:
    void on_pluginListReady();
    void on_log(QString text);
    void on_rebuildUI();

private slots:
    void on_btnDone_clicked();
    void on_btn_clicked(bool checked);

protected:
    void addPluginUI(Plugin *p);

private:
    Ui::DlgPlugins *ui;
    MainWindow *main_;
    QList<Plugin*> uiPlugins_;
    QMap<QWidget*,Plugin*> widgetToPlugin_;

};

#endif // DLGPLUGINS_H
