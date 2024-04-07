#ifndef PAIRMODEL_H
#define PAIRMODEL_H
#include "../controls_global.h"
#include <QAbstractListModel>
#include <QStringList>

typedef QPair<QVariant, QVariant> DataPair;

class CONTROLS_EXPORT PairModel : public QAbstractListModel
{
    Q_OBJECT    
    QList< DataPair > m_content;

public:

    explicit PairModel(QObject *parent = nullptr);
    explicit PairModel(const QList<DataPair>& values, QObject *parent = nullptr);

    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    void setContent(const QList<DataPair>& values);
    void setContent(const QStringList& values);

    QModelIndex findValue(const QVariant& value);

private:

};

#endif // PAIRMODEL_H
