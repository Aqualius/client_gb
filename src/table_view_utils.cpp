#include "table_view_utils.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QMimeData>
#include <QRegExp>
#include <QSqlQuery>
#include <QXmlStreamWriter>

#include "delegate_expenses_part.h"
#include "expenses_part.h"
#include "sql_queries.h"

void TableViewUtils::applyStyle(QTableView *w)
{
    auto copyAction = new QAction("Копировать", w);
    copyAction->setShortcut((QKeySequence(Qt::CTRL + Qt::Key_C)));
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(copyAction, &QAction::triggered, [copyAction, w](bool) { copySelectionToClipboard(w); });
    w->addAction(copyAction);

    auto selectAllAction = new QAction("Выбрать все", w);
    selectAllAction->setShortcut((QKeySequence(Qt::CTRL + Qt::Key_A)));
    selectAllAction->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(selectAllAction, &QAction::triggered, [selectAllAction, w](bool) {
        auto model = w->model();
        do
        {
        } while (model->canFetchMore(model->index(-1, -1)) && (model->fetchMore(model->index(-1, -1)), true));
        w->selectAll();
    });
    w->addAction(selectAllAction);
}

void TableViewUtils::copySelectionToClipboard(QTableView *view, Qt::ItemDataRole role)
{
    if (!view->model())
    {
        return;
    }

    int min_col = view->model()->columnCount() - 1;
    int max_col = 0;

    int min_row = view->model()->rowCount() - 1;
    int max_row = 0;

    QString textData;

    for (auto &sel : view->selectionModel()->selection())
    {  // Дыры в выделении не разрешены, только сплошное выделение
        min_col = qMin(min_col, sel.left());
        max_col = qMax(max_col, sel.right());
        min_row = qMin(min_row, sel.top());
        max_row = qMax(max_row, sel.bottom());
    }

    auto orderedIndexes = orderedLogicalIndexes(view->horizontalHeader());
    QMutableListIterator<int> it(orderedIndexes);
    while (it.hasNext())
    {
        int n = it.next();
        if (n > max_col || n < min_col)
        {
            it.remove();
        }
    }

    QString html;
    QXmlStreamWriter writer(&html);
    writer.writeStartElement("html");
    /**/ writer.writeStartElement("head");
    /****/ writer.writeStartElement("meta");
    /****/ writer.writeAttribute("HTTP-EQUIV", "Content-Type");
    /****/ writer.writeAttribute("CONTENT", "text/html;charset=UTF-8");
    /****/ writer.writeEndElement();  // meta
    /****/ writer.writeStartElement("style");
    /****/ writer.writeComment(R"( .xl63{mso-number-format:"\@";} )");  // Текстовый формат для MS Excel
    /****/ writer.writeEndElement();                                    // style
    /**/ writer.writeEndElement();                                        // head
    /**/ writer.writeStartElement("body");
    /****/ writer.writeStartElement("table");
    /****/ writer.writeAttribute("BORDER", "1");
    /****/ writer.writeAttribute("CELLSPACING", "0");
    /******/ writer.writeStartElement("thead");
    /********/ writer.writeStartElement("tr");
    for (int col : qAsConst(orderedIndexes))
    {
        writer.writeStartElement("th");
        writer.writeAttribute("BGCOLOR", QString("#%1").arg(0, 0, 16));
        writer.writeCharacters(view->model()->headerData(col, Qt::Horizontal).toString());
        writer.writeEndElement();  // th
    }
    /********/ writer.writeEndElement();  // tr
    /******/ writer.writeEndElement();    // thead
    /******/ writer.writeStartElement("tbody");
    for (int row = min_row; row <= max_row; ++row)
    {
        writer.writeStartElement("tr");
        for (int col : qAsConst(orderedIndexes))
        {
            QVariant data = view->model()->index(row, col).data(role);
            QString dataStr = data.toString();

            writer.writeStartElement("td");
            if (data.type() == QVariant::String)
            {
                writer.writeAttribute("class", "xl63");
            }
            else if (data.type() == QVariant::Int || data.type() == QVariant::Double ||
                     data.type() == QVariant::LongLong)
            {
                dataStr = QString::number(data.toDouble(), 'f', 2);
            }
            if (view->model()->headerData(col, Qt::Horizontal).toString() == "Часть расходов")
            {
                if (ExpensesPart::names.contains(dataStr))
                {
                    dataStr = ExpensesPart::names[dataStr];
                }
            }

            writer.writeCharacters(dataStr);
            writer.writeEndElement();  // td
            textData += dataStr.replace('\t', "    ");

            if (col != orderedIndexes.last())
            {
                textData += '\t';
            }
        }
        if (row != max_row)
        {
            textData += '\n';
        }
        writer.writeEndElement();  // tr
    }
    /******/ writer.writeEndElement();  // tbody
    /****/ writer.writeEndElement();    // table
    /**/ writer.writeEndElement();        // body
    writer.writeEndElement();           // html
    auto mime = new QMimeData;
    mime->setHtml(html);
    mime->setText(textData);
    QApplication::clipboard()->setMimeData(mime);
}

QList<int> TableViewUtils::orderedLogicalIndexes(QHeaderView *tableHeader)
{
    QList<int> orderedIndexes;
    for (int col = 0; col < tableHeader->count(); col++)
    {
        int logicalIndex = tableHeader->logicalIndex(col);
        if (!tableHeader->isSectionHidden(logicalIndex))
        {
            orderedIndexes << logicalIndex;
        }
    }
    return orderedIndexes;
}
