#ifndef BASICFILEVIEWER_H
#define BASICFILEVIEWER_H

#include <QDialog>

namespace Ui {
class BasicFileViewer;
}

class BasicFileViewer : public QDialog
{
    Q_OBJECT

public:
    explicit BasicFileViewer(QWidget *parent = nullptr, QString logDir = QString(), QString extractedFile = QString());
    ~BasicFileViewer();

private slots:
    void on_btnDone_clicked();

private:
    Ui::BasicFileViewer *ui;
    QString logDir_;
    QString extractedFile_;
    QString extractedFilePath_;

};

#endif // BASICFILEVIEWER_H
