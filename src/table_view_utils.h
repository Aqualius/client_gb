#ifndef TABLEVIEWUTILS_H
#define TABLEVIEWUTILS_H

#include <QTableView>

class TableViewUtils
{
public:
    static void applyStyle(QTableView *w);

    static void copySelectionToClipboard(QTableView *view, Qt::ItemDataRole role = Qt::DisplayRole);

    static QList<int> orderedLogicalIndexes(QHeaderView *tableHeader);
};

#endif  // TABLEVIEWUTILS_H
