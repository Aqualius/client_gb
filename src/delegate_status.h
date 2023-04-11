#ifndef DELEGATESTATUS_H
#define DELEGATESTATUS_H

#include <QStyledItemDelegate>

class DelegateStatus : public QStyledItemDelegate
{
public:
    explicit DelegateStatus(QObject *parent = nullptr);

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif  // DELEGATESTATUS_H
