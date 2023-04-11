#ifndef LIMITEDTRIMMERTEXTDELEGATE_HPP
#define LIMITEDTRIMMERTEXTDELEGATE_HPP

#include <QObject>
#include <QStyledItemDelegate>

class DelegateLimitedTrimmedText : public QStyledItemDelegate
{
public:
    DelegateLimitedTrimmedText(int maxLength, QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    int m_maxLength;
};

#endif  // LIMITEDTRIMMERTEXTDELEGATE_HPP
