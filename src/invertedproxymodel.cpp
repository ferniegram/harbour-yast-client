#include "invertedproxymodel.h"

#define DEBUG_MODULE InvertedProxyModel
#include "debuglog.h"

InvertedProxyModel::InvertedProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
}

void InvertedProxyModel::setSource(QObject *model) {
    setSourceModel(qobject_cast<QAbstractItemModel*>(model));
}

void InvertedProxyModel::setSourceModel(QAbstractItemModel *model) {
    if (sourceModel() != model) {
        QSortFilterProxyModel::setSourceModel(model);
        emit sourceChanged();
    }
}

QModelIndex InvertedProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
    if (!sourceIndex.isValid()) return QModelIndex();
    return index(rowCount() - sourceIndex.row() - 1, sourceIndex.column());
}

QModelIndex InvertedProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
    if (!proxyIndex.isValid()) return QModelIndex();
    return index(rowCount() - proxyIndex.row() - 1, proxyIndex.column());
}
