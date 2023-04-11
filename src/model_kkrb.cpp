#include "model_kkrb.h"

#include <QBrush>
#include <QColor>

ModelKKRB::ModelKKRB(QObject *parent) : QSqlQueryModel(parent)
{
}

QVariant ModelKKRB::data(const QModelIndex &itemIndex, int role) const
{
    if (role == Qt::BackgroundRole)
    {
        int checker = data(index(itemIndex.row(), 2), Qt::EditRole).toInt();

        if (checker == 0)
        {
            return QBrush(QColor(255, 200, 200));
        }
        else if (checker == 1)
        {
            return QBrush(QColor(200, 255, 200));
        }
    }
    else if (role == Qt::DisplayRole)
    {
        if (itemIndex.column() == 2)
        {
            int status = data(index(itemIndex.row(), 2), Qt::EditRole).toInt();
            return status ? "Verified" : "Unverified";
        }
    }
    return QSqlQueryModel::data(itemIndex, role);
}
