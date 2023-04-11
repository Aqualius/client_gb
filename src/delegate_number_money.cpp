#include "delegate_number_money.h"

DelegateNumberMoney::DelegateNumberMoney(QObject *parent) : QStyledItemDelegate(parent)
{
}

QString DelegateNumberMoney::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::Double || value.type() == QVariant::Int || value.type() == QVariant::LongLong)
    {
        return getMoneyNumber(value);
    }
    return QStyledItemDelegate::displayText(value, locale);
}

void DelegateNumberMoney::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}

QString getMoneyNumber(const QVariant &value)
{
    QString num = QString::number(value.toDouble(), 'f', 2);
    int shift = (value.toDouble() < 0) ? 1 : 0;
    for (int i = num.size() - 6; i >= 1 + shift; i -= 3)
    {
        num.insert(i, ' ');
    }
    return num;
}
