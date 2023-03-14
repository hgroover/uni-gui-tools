#ifndef SORTABLELISTWIDGETITEM_H
#define SORTABLELISTWIDGETITEM_H

#include "sortablelistwidget.h"

#include <QListWidgetItem>
#include <QFileInfo>

class SortableListWidgetItem : public QListWidgetItem
{
public:
    SortableListWidgetItem(SortableListWidget *parent = nullptr, int type = Type);

    SortableListWidgetItem(const QString &text, SortableListWidget *parent = nullptr, int type = Type);

    SortableListWidgetItem(const QIcon &icon, const QString &text, SortableListWidget *parent = nullptr, int type = Type);

    SortableListWidgetItem(const SortableListWidgetItem &other);

    virtual bool operator<(const SortableListWidgetItem &other) const;

    void setFileInfo(QFileInfo& fi) { fi_ = fi; }
    static void setSortOption(QString option);

protected:
    QFileInfo fi_;
    static QString sortOption_;

    QString dtSortKey() const;

};

#endif // SORTABLELISTWIDGETITEM_H
