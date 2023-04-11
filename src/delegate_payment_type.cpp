#include "delegate_payment_type.h"

static const QMap<QString, QString> payment_type_names = { { "FUND", "Финансирование" }, { "REFUND", "Возврат" } };

DelegatePaymentType::DelegatePaymentType(QObject *parent) : QStyledItemDelegate(parent)
{
}

QString DelegatePaymentType::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::String)
    {
        if (payment_type_names.contains(value.toString()))
        {
            return payment_type_names[value.toString()];
        }
    }
    return QStyledItemDelegate::displayText(value, locale);
}

void DelegatePaymentType::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}
