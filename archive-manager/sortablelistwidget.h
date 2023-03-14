#ifndef SORTABLELISTWIDGET_H
#define SORTABLELISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QListWidget>

class SortableListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SortableListWidget(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // SORTABLELISTWIDGET_H
