#ifndef DELEGATEEXPENSESPART_H
#define DELEGATEEXPENSESPART_H

#include <QStyledItemDelegate>

class DelegateExpensesPart : public QStyledItemDelegate
{
public:
    DelegateExpensesPart(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif  // DELEGATEEXPENSESPART_H
