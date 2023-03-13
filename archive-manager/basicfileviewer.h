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

    typedef enum {
        FT_PLAINTEXT,
        FT_HTML,
        FT_JSON,
        FT_BINARY,
        FT_UNKNOWN
    } FileType_t;

    FileType_t detectedFileType() { return detected_; }
private slots:
    void on_btnDone_clicked();

private:
    Ui::BasicFileViewer *ui;
    QString logDir_;
    QString extractedFile_;
    QString extractedFilePath_;
    FileType_t detected_;
};

#endif // BASICFILEVIEWER_H
