#include "delegate_readonly.h"

DelegateReadOnly::DelegateReadOnly(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *DelegateReadOnly::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return 0;
}

void DelegateReadOnly::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}
