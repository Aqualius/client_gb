#ifndef DELEGATE_NOT_EDITABLE_H
#define DELEGATE_NOT_EDITABLE_H

#include <QStyledItemDelegate>

class DelegateReadOnly : public QStyledItemDelegate
{
public:
    DelegateReadOnly(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

#endif  // DELEGATE_NOT_EDITABLE_H
