#ifndef DELEGATENUMBERMONEY_H
#define DELEGATENUMBERMONEY_H

#include <QStyledItemDelegate>

class DelegateNumberMoney : public QStyledItemDelegate
{
public:
    DelegateNumberMoney(QObject *parent = nullptr);

    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

QString getMoneyNumber(const QVariant &value);

#endif  // DELEGATENUMBERMONEY_H
