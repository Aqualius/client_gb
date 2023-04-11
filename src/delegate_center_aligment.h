#ifndef DELEGATECENTERALIGMENT_H
#define DELEGATECENTERALIGMENT_H

#include <QStyledItemDelegate>

class DelegateCenterAligment : public QStyledItemDelegate
{
public:
    explicit DelegateCenterAligment(QObject *parent = nullptr);

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

#endif  // DELEGATECENTERALIGMENT_H
