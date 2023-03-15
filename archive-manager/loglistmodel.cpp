#include "loglistmodel.h"

#include "archive-manager-globals.h"

#include <QDebug>
#include <QDateTime>

LogListModel::LogListModel(const QFileInfoList &afi, QObject *parent)
    : QAbstractListModel(parent),
      sortDescending_(true),
      sortByName_(false)
{
    setFileInfoList(afi);
}

int LogListModel::rowCount(const QModelIndex &parent) const
{
    return aFileInfo_.size();
}

QVariant LogListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        qDebug() << "invalid index";
        return QVariant();
    }

    if (index.row() >= aFileInfo_.size() || index.row() >= gradedIndices_.size())
    {
        qDebug() << "invalid row" << index.row();
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
        {
            //qDebug().noquote() << "file" << aFileInfo_.at(index.row()).fileName();
            return aFileInfo_.at(gradedIndices_[index.row()]).fileName();
        }
        break;

    case Qt::DecorationRole:
        //        The data to be rendered as a decoration in the form of an icon. (QColor, QIcon or QPixmap)
    case Qt::EditRole:
        //        The data in a form suitable for editing in an editor. (QString)
    case Qt::ToolTipRole:
        //        The data displayed in the item's tooltip. (QString)
    case Qt::StatusTipRole:
        //        The data displayed in the status bar. (QString)
    case Qt::WhatsThisRole:
        //        The data displayed for the item in "What's This?" mode. (QString)
    case Qt::SizeHintRole:
        //        The size hint for the item that will be supplied to views. (QSize)
    default:
        break;
    }
    //qDebug().noquote() << "unknown role" << role;
    return QVariant();
}

void LogListModel::setFileInfoList(const QFileInfoList &afi)
{
    aFileInfo_ = afi;
    mapFileNameToIndex_.clear();
    gradeVector_.clear();
    gradedIndices_.clear();
    qDebug().noquote() << "setFileInfoList count" << afi.size();
    for (int n = 0; n < afi.size(); n++)
    {
        mapFileNameToIndex_[afi[n].fileName()] = n;
        setGradeEntry(n);
    }
    calculateGradeIndices();
    emit dataChanged(index(0,0), index(afi.size()-1, 0));
}

void LogListModel::rebuildGradeVector()
{
    gradeVector_.clear();
    gradedIndices_.clear();
    qDebug().noquote() << "rebuildGV";
    for (int n = 0; n < aFileInfo_.size(); n++)
    {
        setGradeEntry(n);
    }
    calculateGradeIndices();
    emit dataChanged(index(0,0), index(aFileInfo_.size()-1, 0));
}

void LogListModel::addFileInfo(const QFileInfo &fi)
{
    int newIndex = aFileInfo_.size();
    aFileInfo_.append(fi);
    mapFileNameToIndex_[fi.fileName()] = newIndex;
    setGradeEntry(newIndex);
    calculateGradeIndices();
    emit dataChanged(index(newIndex,0), index(newIndex,0));
}

QModelIndex LogListModel::findByFile(QString fileName) const
{
    if (mapFileNameToIndex_.contains(fileName))
    {
        return index(mapFileNameToIndex_[fileName], 0);
    }
    return QModelIndex();
}

void LogListModel::setGradeEntry(int index)
{
    // Handle sort options
    QString s;
    if (index >= 0 && index < aFileInfo_.size())
    {
        if (sortByName_)
        {
            s = aFileInfo_[index].fileName();
        }
        else
        {
            s = aFileInfo_[index].lastModified().toString("yyyyMMdd HHmmss");
        }
    }
    gradeVector_.insert(s, index);
}

void LogListModel::calculateGradeIndices()
{
    gradedIndices_.clear();
    QMultiMap<QString,int>::iterator i;
    for (i = gradeVector_.begin(); i != gradeVector_.end(); i++)
    {
        qDebug().noquote() << "cgi[" << gradedIndices_.size() << ']' << i.key() << i.value();
        // To sort descendingly, push the oldest entries to the front so we end up pushing newest last
        if (sortDescending_)
            gradedIndices_.push_front(i.value());
        else
            gradedIndices_.push_back(i.value());
    }
}

void LogListModel::on_sortOrder(QString sortOrder)
{
    rawSortOrder_ = sortOrder;
    // Process ascending / descending
    sortDescending_ = (sortOrder.contains("Descending"));
    sortByName_ = (sortOrder.startsWith("name"));
    qDebug().noquote() << "Sort order:" << sortOrder;
    rebuildGradeVector();
}
