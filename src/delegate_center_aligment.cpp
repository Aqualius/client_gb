#include "delegate_center_aligment.h"

DelegateCenterAligment::DelegateCenterAligment(QObject *parent) : QStyledItemDelegate(parent)
{
}

void DelegateCenterAligment::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}
