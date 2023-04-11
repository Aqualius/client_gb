#include "delegate_expenses_part.h"

#include <QComboBox>
#include <QDebug>
#include <QHeaderView>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QTableView>

#include "expenses_part.h"

DelegateExpensesPart::DelegateExpensesPart(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *DelegateExpensesPart::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                            const QModelIndex &index) const
{
    const int width = 170;
    QComboBox *rComboBox = new QComboBox(parent);
    rComboBox->view()->setMinimumWidth(width);

    QMap<QString, QString>::const_iterator i = ExpensesPart::names.constBegin();
    while (i != ExpensesPart::names.constEnd())
    {
        rComboBox->addItem(i.value(), i.key());
        i++;
    }

    QString currentText = index.data(Qt::EditRole).toString();
    int idx = rComboBox->findData(currentText);
    if (idx > -1)
    {
        rComboBox->setCurrentIndex(idx);
    }
    return rComboBox;
}

QString DelegateExpensesPart::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::String)
    {
        if (ExpensesPart::names.contains(value.toString()))
        {
            return ExpensesPart::names[value.toString()];
        }
    }
    return QStyledItemDelegate::displayText(value, locale);
}

void DelegateExpensesPart::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::initStyleOption(option, index);
}

void DelegateExpensesPart::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
    {
        QVariant data = cb->currentData(Qt::UserRole);
        model->setData(index, data, Qt::EditRole);
    }
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
