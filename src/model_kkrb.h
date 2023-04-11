#ifndef MODEL_KKRB_H
#define MODEL_KKRB_H

#include <QSqlQueryModel>

class ModelKKRB : public QSqlQueryModel
{
public:
    explicit ModelKKRB(QObject *parent = nullptr);
    QVariant data(const QModelIndex &itemIndex, int role = Qt::DisplayRole) const override;
};

#endif  // MODEL_KKRB_H
