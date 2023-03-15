#ifndef LOGLISTMODEL_H
#define LOGLISTMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QFileInfoList>
#include <QMap>
#include <QMultiMap>

class LogListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    LogListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent),
          sortDescending_(true),
          sortByName_(false)
    {}

    LogListModel(const QFileInfoList &afi, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    /* QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override; */

    void setFileInfoList(const QFileInfoList &afi);
    void addFileInfo(const QFileInfo &fi);
    QModelIndex findByFile(QString fileName) const;
    QString fileFromIndex(QModelIndex index) const;

public slots:
    void on_sortOrder(QString sortOrder);

protected:
    QFileInfoList aFileInfo_;
    QMap<QString,int> mapFileNameToIndex_;
    QMultiMap<QString,int> gradeVector_;
    QList<int> gradedIndices_;
    QString rawSortOrder_;
    bool sortDescending_;
    bool sortByName_;

    void setGradeEntry(int index);
    void rebuildGradeVector();
    void calculateGradeIndices();

};

#endif // LOGLISTMODEL_H
