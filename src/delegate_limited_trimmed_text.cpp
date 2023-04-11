#include "delegate_limited_trimmed_text.h"

#include <QLineEdit>

DelegateLimitedTrimmedText::DelegateLimitedTrimmedText(int maxLength, QObject *parent)
    : QStyledItemDelegate(parent), m_maxLength(maxLength)
{
}

QWidget *DelegateLimitedTrimmedText::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/,
                                                  const QModelIndex & /*index*/) const
{
    auto lineEdit = new QLineEdit(parent);
    lineEdit->setMaxLength(m_maxLength);
    return lineEdit;
}

void DelegateLimitedTrimmedText::setModelData(QWidget *editor, QAbstractItemModel *model,
                                              const QModelIndex &index) const
{
    auto lineEdit = static_cast<QLineEdit *>(editor);
    QString text = lineEdit->text().simplified();
    model->setData(index, text.isEmpty() ? QVariant() : text);
}
