#include "sortablelistwidgetitem.h"
#include "mainwindow.h"

#include <QDateTime>

QString SortableListWidgetItem::sortOption_;

SortableListWidgetItem::SortableListWidgetItem(SortableListWidget *parent, int type)
    : QListWidgetItem (parent, type),
      fi_()
{

}

SortableListWidgetItem::SortableListWidgetItem(const QString &text, SortableListWidget *parent, int type)
    : QListWidgetItem (text, parent, type),
      fi_()
{

}

SortableListWidgetItem::SortableListWidgetItem(const QIcon &icon, const QString &text, SortableListWidget *parent, int type)
    : QListWidgetItem (icon, text, parent, type),
      fi_()
{}

SortableListWidgetItem::SortableListWidgetItem(const SortableListWidgetItem &other)
    : QListWidgetItem (other),
      fi_()
{}

bool SortableListWidgetItem::operator<(const SortableListWidgetItem &other) const
{
    // Key is datetime, and later should come first by default
    if (sortOption_ == "da")
    {
        return dtSortKey() >= other.dtSortKey();
    }
    bool isLess = (dtSortKey() >= other.dtSortKey());
    if (MainWindow::g_verbose > 0)
    {
        qDebug().noquote() << "lt" << isLess << dtSortKey() << "<->" << other.dtSortKey();
    }
    return isLess;
}

QString SortableListWidgetItem::dtSortKey() const
{
    QString s("0000-00-00 00:00:00");
    s = fi_.lastModified().toString("yyyyMMdd HHmmss");
    return s;
}

void SortableListWidgetItem::setSortOption(QString option)
{
    if (option == "dateAscending") sortOption_ = "da";
    else if (option == "dateDescending") sortOption_ = "dd";
    else sortOption_.clear();
}
