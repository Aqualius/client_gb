#include "delegate_status.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>

DelegateStatus::DelegateStatus(QObject *parent) : QStyledItemDelegate(parent)
{
}

void DelegateStatus::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}

void DelegateStatus::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt{ option };
    initStyleOption(&opt, index);

    QColor color(
        index.model()->data(index.model()->index(index.row(), index.column() + 1), Qt::DisplayRole).toString());
    opt.palette.setColor(QPalette::Text, color);
    opt.font.setBold(true);

    QStyledItemDelegate::paint(painter, opt, index);
}
