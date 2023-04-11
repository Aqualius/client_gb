#ifndef DELEGATEPAYMENTTYPE_H
#define DELEGATEPAYMENTTYPE_H

#include <QStyledItemDelegate>

class DelegatePaymentType : public QStyledItemDelegate
{
public:
    DelegatePaymentType(QObject *parent = nullptr);

    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
};

#endif  // DELEGATEPAYMENTTYPE_H
