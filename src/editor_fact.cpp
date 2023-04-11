#include "editor_fact.h"

#include <QAction>
#include <QClipboard>
#include <QDate>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>

#include "delegate_center_aligment.h"
#include "delegate_expenses_part.h"
#include "delegate_limited_trimmed_text.h"
#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "expenses_part.h"
#include "table_view_utils.h"
#include "ui_editor_fact.h"

struct ColumnData
{
    QString name;
    QString type;
    int width;
};

EditorFact::EditorFact(int id, bool read_only, QWidget *parent) : QWidget(parent), mIdDoc(id), ui(new Ui::EditorFact)
{
    ui->setupUi(this);
    ui->cb_kubp->setFocus();

    popUp = new PopUp(this);

    setTypeDoc();
    setExpensesPart();

    setWindowTitle(QString("Заявка №%1").arg(getDocNum()));
    setCurrentMonth();

    dirty = false;
    if (checkIdDoc())
    {
        int res = QMessageBox::question(this, "Найден открытый редактор",
                                        "Хотите продолжить работу с предыдущим редактором?");
        if (res == QMessageBox::No)
        {
            insertToExpensesEditor();
        }
        else
        {
            dirty = true;
        }
    }
    else
    {
        insertToExpensesEditor();
    }

    setDropDownTable(ui->cb_kubp, Queries::kubp_fact);

    mBalanceModel = new ModelKKRB(this);
    ui->balance_sum->setModel(mBalanceModel);
    ui->balance_sum->setItemDelegateForColumn(0, new DelegateNumberMoney(this));

    ////////////////////////////////////////
    /////////// Основная таблица ///////////
    ////////////////////////////////////////
    QList<ColumnData> column_data = { { "id", "hidden", 0 },
                                      { "document_id", "hidden", 0 },
                                      { "КРП", "center_aligment", 39 },
                                      { "КЦС", "center_aligment", 49 },
                                      { "КВР", "center_aligment", 39 },
                                      { "КОСГУ", "center_aligment", 45 },
                                      { "КУБП", "center_aligment", 49 },
                                      { "Примечание", "limited_text", 100 },
                                      { "Направление расходов", "limited_text", 400 },
                                      { "Часть расходов", "exp_part", 70 },
                                      { "Сумма", "number", 0 } };

    if (read_only)
    {
        ui->cb_kubp->setEnabled(false);
        ui->cb_notes->setEnabled(false);
        ui->cb_direction->setEnabled(false);
        ui->cb_exp_part->setEnabled(false);
        ui->add_kkrb->setEnabled(false);
        ui->del_str_fact_table->setEnabled(false);
        ui->resize_column_button->setEnabled(false);
        ui->save_button->setEnabled(false);
        ui->fact_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        auto pasteAction = new QAction("Вставить", ui->fact_table);
        pasteAction->setShortcut((QKeySequence(Qt::CTRL + Qt::Key_V)));
        pasteAction->setShortcutContext(Qt::WidgetShortcut);
        connect(pasteAction, &QAction::triggered, this, [=](bool) { pasteFromClipboard(ui->fact_table); });
        ui->fact_table->addAction(pasteAction);
    }

    mFactModel = new QSqlTableModel(this);
    mFactModel->setTable("expenses_editor_fact");
    mFactModel->setSort(0, Qt::DescendingOrder);
    mFactModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    mFactModel->select();
    connect(mFactModel, &QSqlTableModel::dataChanged, this, &EditorFact::dataChanged);

    ui->fact_table->setModel(mFactModel);
    ui->fact_table->setAlternatingRowColors(true);

    for (int i = 0; i < mFactModel->columnCount(); i++)
    {
        mFactModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        ui->column_sum->insertColumn(i);
        ui->filter_row->insertColumn(i);
        QString type = column_data[i].type;
        if (type == "hidden")
        {
            ui->fact_table->hideColumn(i);
            ui->column_sum->hideColumn(i);
            ui->filter_row->hideColumn(i);
        }
        else if (type == "center_aligment")
        {
            ui->fact_table->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
            ui->filter_row->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
        }
        else if (type == "limited_text")
        {
            ui->fact_table->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(400, this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(400, this));
            ui->filter_row->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(400, this));
            ui->fact_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            ui->column_sum->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            ui->filter_row->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        }
        else if (type == "number")
        {
            ui->fact_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItem(0, i, new QTableWidgetItem("0"));
            ui->column_sum->resizeColumnToContents(i);
        }
        else if (type == "exp_part")
        {
            ui->fact_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
    }

    on_filter_row_cellChanged(0, 0);

    ////////////////////////////////////////
    /////////////// Коннекты ////////////////
    ////////////////////////////////////////

    TableViewUtils::applyStyle(ui->fact_table);

    totalsDebounceTimer = new QTimer(this);
    totalsDebounceTimer->setInterval(400);
    totalsDebounceTimer->setSingleShot(true);
    connect(totalsDebounceTimer, &QTimer::timeout, this, &EditorFact::updateSum);
    totalsDebounceTimer->start();

    ui->balance_sum->setAlternatingRowColors(true);
    ui->balance_sum->horizontalHeader()->setVisible(true);
    ui->fact_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(ui->fact_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->column_sum->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->fact_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->filter_row->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->column_sum->horizontalScrollBar(), &QScrollBar::valueChanged, ui->fact_table->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->filter_row->horizontalScrollBar(), &QScrollBar::valueChanged, ui->fact_table->horizontalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->fact_table->verticalScrollBar(), &QScrollBar::valueChanged, ui->balance_sum->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->balance_sum->verticalScrollBar(), &QScrollBar::valueChanged, ui->fact_table->verticalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->fact_table->horizontalHeader(), &QHeaderView::sectionResized, this,
            [this](int logicalIndex) { resizeColumnSumWidth(logicalIndex); });

    resizeRowHeight();

    rightClickMenu = new QMenu(this);
    ui->fact_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fact_table, &QTableView::customContextMenuRequested, this, &EditorFact::customContextMenuRequested);

    QApplication::restoreOverrideCursor();
}

EditorFact::~EditorFact()
{
    delete ui;
}

int EditorFact::getDocNum()
{
    QSqlQuery query;
    query.prepare(Queries::num_from_fact_doc);
    query.bindValue(":id", mIdDoc);
    query.exec();
    if (query.next())
    {
        return query.value("doc_number").toInt();
    }
    return 0;
}

void EditorFact::setCurrentMonth()
{
    mCurrentMonth = QDate::currentDate().month();
}

void EditorFact::insertToExpensesEditor()
{
    QSqlQuery query;
    query.prepare(Queries::delete_in_exp_fact);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Ошибка очистки редактора. Детали:\n" + query.lastError().text());
        popUp->show();
        return;
    }
    query.prepare(Queries::insert_in_exp_fact);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        qCritical() << query.lastError();
    }
}

void EditorFact::setDropDownTable(QComboBox *rComboBox, const QString &rQuery)
{
    QSqlQuery query;
    query.prepare(rQuery);
    query.bindValue(":document_id", mIdDoc);
    query.bindValue(":kubp", ui->cb_kubp->currentText());
    query.bindValue(":krp", ui->cb_krp->currentText());
    query.bindValue(":kcs", ui->cb_kcs->currentText());
    query.bindValue(":kvr", ui->cb_kvr->currentText());
    query.bindValue(":kosgu", ui->cb_kosgu->currentText());
    query.bindValue(":month_n", mCurrentMonth);
    query.exec();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(query);
    model->setHeaderData(0, Qt::Horizontal, "Код");
    model->setHeaderData(1, Qt::Horizontal, "Наименование");

    QTableView *table_view = new QTableView(this);
    rComboBox->setView(table_view);
    rComboBox->setModel(model);

    // скрыть номера строк
    table_view->verticalHeader()->hide();

    // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
    table_view->setMinimumWidth(mWidthDropDown);
    for (int i = 0; i < model->rowCount(); i++)
    {
        table_view->setRowHeight(i, 11);
    }
    table_view->setColumnWidth(0, 50);
    table_view->setColumnWidth(1, mWidthDropDown - table_view->columnWidth(0) - 17);

    // выбор всей строки, а не ячейки
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    rComboBox->setModelColumn(0);
    rComboBox->setCurrentIndex(-1);
}

void EditorFact::resizeColSum()
{
    for (int i = 2; i < mFactModel->columnCount(); i++)
    {
        ui->column_sum->resizeColumnToContents(i);
        ui->fact_table->resizeColumnToContents(i);
        int width = ui->column_sum->columnWidth(i) > ui->fact_table->columnWidth(i) ? ui->column_sum->columnWidth(i) :
                                                                                      ui->fact_table->columnWidth(i);
        QTableWidgetItem *item = ui->column_sum->item(0, i);
        if (item != nullptr)
        {
            width += 10;
        }
        ui->column_sum->setColumnWidth(i, width);
        ui->filter_row->setColumnWidth(i, width);
        ui->fact_table->setColumnWidth(i, width);
    }
}

void EditorFact::resizeColumnSumWidth(int ind)
{
    ui->column_sum->setColumnWidth(ind, ui->fact_table->columnWidth(ind));
    ui->filter_row->setColumnWidth(ind, ui->fact_table->columnWidth(ind));
}

void EditorFact::resizeRowHeight()
{
    ui->peg->setMaximumWidth(ui->fact_table->verticalHeader()->width());
    ui->peg_2->setMaximumWidth(ui->fact_table->verticalHeader()->width());
}

void EditorFact::setExpensesPart()
{
    ui->cb_exp_part->clear();
    QMap<QString, QString>::const_iterator i = ExpensesPart::names.constBegin();
    while (i != ExpensesPart::names.constEnd())
    {
        ui->cb_exp_part->insertItem(-1, i.value());
        ++i;
    }
    ui->cb_exp_part->setCurrentIndex(-1);
}

void EditorFact::setTypeDoc()
{
    QSqlQuery query;
    query.prepare(Queries::get_type_doc_fact);
    query.bindValue(":id", mIdDoc);
    query.exec();
    while (query.next())
    {
        mRefund = query.value(0) == "REFUND";
    }
}

QString EditorFact::getExpensesPart() const
{
    return ExpensesPart::names.key(ui->cb_exp_part->currentText());
}

void EditorFact::updateSum()
{
    if (mFactModel->lastError().type() != QSqlError::NoError)
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Неверный ввод. Детали ошибки:\n" + mFactModel->lastError().text());
        popUp->show();
    }
    QString filter = QString("document_id = %1").arg(mIdDoc);
    for (int i = 0; i < ui->filter_row->columnCount(); i++)
    {
        if (ui->filter_row->item(0, i) == nullptr)
        {
            continue;
        }
        if (ui->filter_row->item(0, i)->text().isEmpty())
        {
            continue;
        }
        filter += QString(" AND eef.%1 LIKE '%%2%'")
                      .arg(mFactModel->record().fieldName(i), ui->filter_row->item(0, i)->text().replace("'", "\""));
    }
    QSqlQuery query;
    query.prepare(mRefund ? Queries::balance_fact_refund.arg(filter) : Queries::balance_plan_n_fact.arg(filter));
    query.bindValue(":id", mIdDoc);
    query.bindValue(":month_n", mCurrentMonth);
    query.exec();
    mBalanceModel->setQuery(query);
    mBalanceModel->setHeaderData(0, Qt::Horizontal, "[Остаток]");
    QSqlQuery query_sum;
    query_sum.prepare(Queries::sum_col_exp_fact.arg(mFilter));
    query_sum.exec();
    if (query_sum.next())
    {
        ui->column_sum->item(0, 10)->setText(getMoneyNumber(query_sum.value(0).toDouble()));
    }
    resizeColSum();
}

bool EditorFact::checkCode()
{
    QSqlQuery query;
    query.prepare(Queries::check_kkrb_exp_fact);
    query.bindValue(":id", mIdDoc);
    query.bindValue(":krp", ui->cb_krp->currentText());
    query.bindValue(":kcs", ui->cb_kcs->currentText());
    query.bindValue(":kvr", ui->cb_kvr->currentText());
    query.bindValue(":kosgu", ui->cb_kosgu->currentText());
    query.bindValue(":kubp", ui->cb_kubp->currentText());
    query.bindValue(":exp_part", ui->cb_exp_part->currentText());
    query.bindValue(":notes", ui->cb_notes->currentText().isEmpty() ? QVariant() : ui->cb_notes->currentText());
    query.exec();
    while (query.next())
    {
        if (query.value(0).toInt() != 0)
        {
            return true;
        }
    }
    return false;
}

void EditorFact::setDirectionExpenditure()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(Queries::direction_fact);
    model->setHeaderData(0, Qt::Horizontal, "Наименование");

    QTableView *table_view = new QTableView(this);
    ui->cb_direction->setView(table_view);
    ui->cb_direction->setModel(model);

    // скрыть номера строк
    table_view->verticalHeader()->hide();

    // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
    table_view->setMinimumWidth(mWidthDropDown);
    for (int i = 0; i < model->rowCount(); i++)
    {
        table_view->setRowHeight(i, 11);
    }
    table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    // выбор всей строки, а не ячейки
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->cb_direction->setModelColumn(0);
    ui->cb_direction->setCurrentIndex(-1);
}

void EditorFact::on_cb_kubp_currentIndexChanged(int index)
{
    ui->cb_kubp->lineEdit()->setStyleSheet("");
    ui->cb_krp->setEnabled(false);
    ui->cb_kcs->setEnabled(false);
    ui->cb_kvr->setEnabled(false);
    ui->cb_kosgu->setEnabled(false);
    ui->cb_krp->setCurrentIndex(-1);
    ui->cb_kcs->setCurrentIndex(-1);
    ui->cb_kvr->setCurrentIndex(-1);
    ui->cb_kosgu->setCurrentIndex(-1);
    ui->cb_notes->setEnabled(false);
    ui->cb_notes->setCurrentIndex(-1);
    ui->cb_exp_part->setEnabled(false);
    ui->cb_exp_part->setCurrentIndex(-1);
    if (index != -1)
    {
        setDropDownTable(ui->cb_krp, Queries::kkrb_krp_fact);
        ui->cb_krp->setEnabled(true);
        ui->cb_krp->setFocus();
    }
}

void EditorFact::on_cb_krp_currentIndexChanged(int index)
{
    ui->cb_krp->lineEdit()->setStyleSheet("");
    if (ui->cb_krp->isEnabled())
    {
        ui->cb_kcs->setEnabled(false);
        ui->cb_kvr->setEnabled(false);
        ui->cb_kosgu->setEnabled(false);
        ui->cb_kcs->setCurrentIndex(-1);
        ui->cb_kvr->setCurrentIndex(-1);
        ui->cb_kosgu->setCurrentIndex(-1);
        ui->cb_notes->setEnabled(false);
        ui->cb_notes->setCurrentIndex(-1);
        ui->cb_exp_part->setEnabled(false);
        ui->cb_exp_part->setCurrentIndex(-1);
        if (index != -1)
        {
            setDropDownTable(ui->cb_kcs, Queries::kkrb_kcs_fact);
            ui->cb_kcs->setEnabled(true);
            ui->cb_kcs->setFocus();
        }
    }
}

void EditorFact::on_cb_kcs_currentIndexChanged(int index)
{
    ui->cb_kcs->lineEdit()->setStyleSheet("");
    if (ui->cb_kcs->isEnabled())
    {
        ui->cb_kvr->setEnabled(false);
        ui->cb_kosgu->setEnabled(false);
        ui->cb_kvr->setCurrentIndex(-1);
        ui->cb_kosgu->setCurrentIndex(-1);
        ui->cb_notes->setEnabled(false);
        ui->cb_notes->setCurrentIndex(-1);
        ui->cb_exp_part->setEnabled(false);
        ui->cb_exp_part->setCurrentIndex(-1);
        if (index != -1)
        {
            setDropDownTable(ui->cb_kvr, Queries::kkrb_kvr_fact);
            ui->cb_kvr->setEnabled(true);
            ui->cb_kvr->setFocus();
        }
    }
}

void EditorFact::on_cb_kvr_currentIndexChanged(int index)
{
    ui->cb_kvr->lineEdit()->setStyleSheet("");
    if (ui->cb_kvr->isEnabled())
    {
        ui->cb_kosgu->setEnabled(false);
        ui->cb_kosgu->setCurrentIndex(-1);
        ui->cb_notes->setEnabled(false);
        ui->cb_notes->setCurrentIndex(-1);
        ui->cb_exp_part->setEnabled(false);
        ui->cb_exp_part->setCurrentIndex(-1);
        if (index != -1)
        {
            setDropDownTable(ui->cb_kosgu, Queries::kkrb_kosgu_fact);
            ui->cb_kosgu->setEnabled(true);
            ui->cb_kosgu->setFocus();
        }
    }
}

void EditorFact::on_cb_kosgu_currentIndexChanged(int index)
{
    ui->cb_kosgu->lineEdit()->setStyleSheet("");
    if (ui->cb_kosgu->isEnabled())
    {
        if (index == -1)
        {
            ui->cb_notes->setEnabled(false);
            ui->cb_notes->setCurrentIndex(-1);
            ui->cb_exp_part->setEnabled(false);
            ui->cb_exp_part->setCurrentIndex(-1);
        }
        else
        {
            // NOTES
            ui->cb_notes->setEnabled(true);
            QSqlQuery query;
            query.prepare(Queries::notes_fact);
            query.bindValue(":document_id", mIdDoc);
            query.bindValue(":kubp", ui->cb_kubp->currentText());
            query.bindValue(":krp", ui->cb_krp->currentText());
            query.bindValue(":kcs", ui->cb_kcs->currentText());
            query.bindValue(":kvr", ui->cb_kvr->currentText());
            query.bindValue(":kosgu", ui->cb_kosgu->currentText());
            query.bindValue(":month_n", mCurrentMonth);
            query.exec();

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery(query);
            model->setHeaderData(0, Qt::Horizontal, "Наименование");

            QTableView *table_view = new QTableView(this);
            ui->cb_notes->setView(table_view);
            ui->cb_notes->setModel(model);

            // скрыть номера строк
            table_view->verticalHeader()->hide();

            // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
            table_view->setMinimumWidth(mWidthDropDown);
            for (int i = 0; i < model->rowCount(); i++)
            {
                table_view->setRowHeight(i, 11);
            }
            table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

            // выбор всей строки, а не ячейки
            table_view->setSelectionMode(QAbstractItemView::SingleSelection);
            table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

            ui->cb_notes->setModelColumn(0);
            ui->cb_notes->setFocus();

            // EXPENSES PART
            ui->cb_exp_part->setEnabled(true);
            query.prepare(Queries::exp_part_fact);
            query.bindValue(":document_id", mIdDoc);
            query.bindValue(":kubp", ui->cb_kubp->currentText());
            query.bindValue(":krp", ui->cb_krp->currentText());
            query.bindValue(":kcs", ui->cb_kcs->currentText());
            query.bindValue(":kvr", ui->cb_kvr->currentText());
            query.bindValue(":kosgu", ui->cb_kosgu->currentText());
            query.bindValue(":notes", ui->cb_notes->currentText().isEmpty() ? QVariant() : ui->cb_notes->currentText());
            query.bindValue(":month_n", mCurrentMonth);
            query.exec();

            QSqlQueryModel *model_ep = new QSqlQueryModel(this);
            model_ep->setQuery(query);
            model_ep->setHeaderData(0, Qt::Horizontal, "Наименование");

            QTableView *table_view_ep = new QTableView(this);
            ui->cb_exp_part->setView(table_view_ep);
            ui->cb_exp_part->setModel(model_ep);

            // скрыть номера строк
            table_view_ep->verticalHeader()->hide();

            // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
            table_view_ep->setMinimumWidth(mWidthDropDown);
            for (int i = 0; i < model_ep->rowCount(); i++)
            {
                table_view_ep->setRowHeight(i, 11);
            }
            table_view_ep->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

            // выбор всей строки, а не ячейки
            table_view_ep->setSelectionMode(QAbstractItemView::SingleSelection);
            table_view_ep->setSelectionBehavior(QAbstractItemView::SelectRows);

            ui->cb_exp_part->setModelColumn(0);
        }
    }
}

void EditorFact::on_cb_notes_currentIndexChanged(int index)
{
    ui->cb_notes->lineEdit()->setStyleSheet("");
    ui->cb_exp_part->setFocus();
}

void EditorFact::on_cb_exp_part_currentIndexChanged(int index)
{
    ui->add_kkrb->setFocus();
}

void EditorFact::on_add_kkrb_clicked()
{
    if (checkCode())
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Такой ККРБ уже есть в заявке!");
        popUp->show();
        return;
    }
    ui->cb_kubp->lineEdit()->setStyleSheet("");
    ui->cb_krp->lineEdit()->setStyleSheet("");
    ui->cb_kcs->lineEdit()->setStyleSheet("");
    ui->cb_kvr->lineEdit()->setStyleSheet("");
    ui->cb_kosgu->lineEdit()->setStyleSheet("");
    if (ui->cb_kubp->findText(ui->cb_kubp->currentText()) == -1)
    {
        ui->cb_kubp->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_krp->findText(ui->cb_krp->currentText()) == -1)
    {
        ui->cb_krp->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_kcs->findText(ui->cb_kcs->currentText()) == -1)
    {
        ui->cb_kcs->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_kvr->findText(ui->cb_kvr->currentText()) == -1)
    {
        ui->cb_kvr->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_kosgu->findText(ui->cb_kosgu->currentText()) == -1)
    {
        ui->cb_kosgu->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_exp_part->currentIndex() == -1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите часть расходов!");
        popUp->show();
        return;
    }

    QSqlRecord record = mFactModel->record();
    record.setValue("document_id", mIdDoc);
    record.setValue("kubp", ui->cb_kubp->currentText());
    record.setValue("krp", ui->cb_krp->currentText());
    record.setValue("kcs", ui->cb_kcs->currentText());
    record.setValue("kvr", ui->cb_kvr->currentText());
    record.setValue("kosgu", ui->cb_kosgu->currentText());
    record.setValue("notes", ui->cb_notes->currentText().isEmpty() ? QVariant() : ui->cb_notes->currentText());
    record.setValue("direction",
                    ui->cb_direction->currentText().isEmpty() ? QVariant() : ui->cb_direction->currentText());
    record.setValue("expenses_part", getExpensesPart());

    record.setGenerated("id", false);
    record.setGenerated("amount", false);

    if (!mFactModel->insertRecord(0, record))
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Не удалось добавить ККРБ в базу данных.");
        popUp->show();
        qCritical() << record << mFactModel->lastError();
        return;
    }
    if (ui->check_clear_kkrb->isChecked())
    {
        ui->cb_kubp->setCurrentIndex(-1);
        ui->cb_direction->setCurrentIndex(-1);
    }
    ui->fact_table->setCurrentIndex(mFactModel->index(0, 10));
    ui->fact_table->setFocus();
}

void EditorFact::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    dirty = true;
    totalsDebounceTimer->start();
}

void EditorFact::closeEvent(QCloseEvent *event)
{
    QSqlQuery query;
    if (dirty)
    {
        int res =
            QMessageBox::question(this, "Сохранение данных", "Обнаружены не сохраненные данные, хотите их сохранить?",
                                  QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard, QMessageBox::Save);

        if (res == QMessageBox::Save)
        {
            if (!saveData())
            {
                QMessageBox::critical(this, "Неудача", "Данные не сохранены. Причина:\n" + mLastError);
                event->ignore();
                return;
            }
        }
        else if (res == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
    }

    query.prepare(Queries::delete_in_exp_fact);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        qCritical() << query.lastError();
        event->ignore();
        return;
    }
}

void EditorFact::on_resize_column_button_clicked()
{
    for (int i = 0; i < mFactModel->columnCount() - 1; i++)
    {
        ui->fact_table->resizeColumnToContents(i);
    }
}

void EditorFact::on_del_str_fact_table_clicked()
{
    QSet<int> set;
    const auto selected_index = ui->fact_table->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selected_index)
    {
        set << index.row();
    }
    if (set.size() < 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите, что вы хотите удалить!");
        popUp->show();
        return;
    }
    int res =
        QMessageBox::question(this, "Подтвердите", QString("Вы точно хотите удалить %1 строк(и)?").arg(set.size()));
    if (res == QMessageBox::Yes)
    {
        QSqlDatabase::database().transaction();
        for (int ind_row : qAsConst(set))
        {
            if (!mFactModel->removeRow(ind_row))
            {
                popUp->setPopupType(Error);
                popUp->setPopupText(QString("Ошибка при удалении строки №%1 из базы данных.").arg(ind_row));
                popUp->show();
                QSqlDatabase::database().rollback();
                qCritical() << mFactModel->lastError();
                return;
            }
            dirty = true;
        }
        QSqlDatabase::database().commit();
        mFactModel->select();
        updateSum();
    }
}

bool EditorFact::checkIdDoc() const
{
    QSqlQuery query;
    query.prepare(Queries::check_id_fact);
    query.bindValue(":id", mIdDoc);
    query.exec();
    return query.next();
}

void EditorFact::on_save_button_clicked()
{
    if (saveData())
    {
        dirty = false;
        popUp->setPopupType(Access);
        popUp->setPopupText("Данные успешно сохранены!");
        popUp->show();
    }
    else
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Данные не сохранены. Причина:\n" + mLastError);
        popUp->show();
    }
}

bool EditorFact::saveData()
{
    if (!checkDuplicate())
    {
        mLastError = "Найдены дубликаты. Проверьте данные!";
        return false;
    }
    if (mRefund)
    {
        for (int i = 0; i < mBalanceModel->rowCount(); i++)
        {
            QVariant balance_data = mBalanceModel->data(mBalanceModel->index(i, 0));
            double amount = mFactModel->data(mFactModel->index(i, 10)).toDouble();
            if (amount >= 0.)
            {
                mLastError = QString("В возвратах сумма не отрицательная в строке %1").arg(i + 1);
                qCritical() << "В возвратах сумма не отрицательная в строке" << i + 1;
                return false;
            }
            if (balance_data.isNull())
            {
                mLastError =
                    QString("Отсутствует финансирование для %1 строки. Проверьте пожалуйста данные").arg(i + 1);
                qCritical() << "Отсутствует финансирование в строке" << i + 1;
                return false;
            }
            if (balance_data.toDouble() < 0.)
            {
                mLastError = QString("Отрицательный остаток в %1 строке. Проверьте пожалуйста данные").arg(i + 1);
                qCritical() << "Отрицательный остаток в строке " << i + 1;
                return false;
            }
        }
    }
    else
    {
        for (int i = 0; i < mBalanceModel->rowCount(); i++)
        {
            QVariant balance_data = mBalanceModel->data(mBalanceModel->index(i, 0));
            double amount = mFactModel->data(mFactModel->index(i, 10)).toDouble();
            if (amount <= 0.)
            {
                mLastError = QString("В финансировании сумма не положительная в строке %1").arg(i + 1);
                qCritical() << "В финансировании сумма не положительная в строке" << i + 1;
                return false;
            }
            if (balance_data.isNull())
            {
                mLastError = QString("Отсутствуют планы для %1 строки. Проверьте пожалуйста данные").arg(i + 1);
                qCritical() << "Отсутствуют планы в строке " << i + 1;
                return false;
            }
            else if (balance_data.toDouble() < 0.)
            {
                mLastError = QString("Отрицательный остаток в %1 строке. Проверьте пожалуйста данные").arg(i + 1);
                qCritical() << "Отрицательный остаток в строке " << i + 1;
                return false;
            }
        }
    }
    QSqlQuery query;
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    query.prepare(Queries::delete_in_fact_data);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        mLastError = "Ошибка SQL запроса: " + query.lastError().text();
        qCritical() << query.lastError();
        db.rollback();
        return false;
    }
    query.prepare(Queries::insert_in_fact_data);
    query.bindValue(":id", mIdDoc);
    query.bindValue(":year", QDate::currentDate().year());
    query.bindValue(":month", QDate::currentDate().month());
    if (!query.exec())
    {
        mLastError = "Ошибка SQL запроса: " + query.lastError().text();
        qCritical() << query.lastError();
        db.rollback();
        return false;
    }
    db.commit();
    emit updateData();
    return true;
}

bool EditorFact::checkDuplicate()
{
    on_filter_clear_clicked();
    QSqlQuery query;
    query.prepare(Queries::check_duplicate_exp_fact);
    query.bindValue(":id", mIdDoc);
    query.exec();
    if (query.next())
    {
        int j = 0;
        for (int i = 0; i < ui->filter_row->columnCount(); i++)
        {
            if (ui->filter_row->isColumnHidden(i) || i == 10)
            {
                continue;
            }
            ui->filter_row->setItem(0, i, new QTableWidgetItem(query.value(j).toString()));
            j++;
        }
        return false;
    }
    return true;
}

void EditorFact::pasteFromClipboard(QTableView *view, Qt::ItemDataRole role)
{
    popUp->setPopupType(Warning);
    QStringList rows = QApplication::clipboard()->text().split('\n', Qt::SkipEmptyParts);

    const QList<QString> exp_part_names_values = ExpensesPart::names.values();
    QSqlDatabase::database().transaction();
    for (int i = 0; i < rows.size(); i++)
    {
        QStringList cols = rows.at(i).split('\t');
        if (cols.size() != 9)
        {
            QString error_str = QString(
                                    "При вставке не то количество столбиков. Должно быть 9, а получили %1. Данные "
                                    "строки:")
                                    .arg(cols.size());
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        for (QString &str : cols)
        {
            str = str.simplified();
        }
        QRegExp re_krp(R"(^\d{4}$)");
        QRegExp re_kcs(R"(^\d{7}$)");
        QRegExp re_kvr(R"(^\d{3}$)");
        QRegExp re_kubp(R"(^\d{5,6}$)");
        QRegExp re_double(R"(^(0|(\-|\+)?([0-9][0-9]*)+([\.,][0-9]{1,2})?)$)");
        if (!re_krp.exactMatch(cols[0]))
        {
            QString error_str = QString("КРП должен состоять из 4 цифр, а было получено %1").arg(cols[0]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        if (!re_kcs.exactMatch(cols[1]))
        {
            QString error_str = QString("КЦС должен состоять из 7 цифр, а было получено %1").arg(cols[1]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        if (!re_kvr.exactMatch(cols[2]))
        {
            QString error_str = QString("КВР должен состоять из 3 цифр, а было получено %1").arg(cols[2]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        if (!re_kvr.exactMatch(cols[3]))
        {
            QString error_str = QString("КОСГУ должен состоять из 3 цифр, а было получено %1").arg(cols[3]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        if (!re_kubp.exactMatch(cols[4]))
        {
            QString error_str = QString("КУБП должен состоять из 5 или 6 цифр, а было получено %1").arg(cols[4]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        if (!exp_part_names_values.contains(cols[7]))
        {
            QString error_str = QString("Часть расходов не верная (%1 нет в базе)").arg(cols[7]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            break;
        }
        else
        {
            cols[7] = ExpensesPart::names.key(cols[7]);
        }
        if (!re_double.exactMatch(cols[8]))
        {
            QString error_str = QString("Сумма должна быть числом, а было получено %1").arg(cols[8]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
        QSqlQuery query;
        query.prepare(Queries::insert_in_exp_fact_manual);
        query.bindValue(":id", mIdDoc);
        query.bindValue(":krp", cols[0]);
        query.bindValue(":kcs", cols[1]);
        query.bindValue(":kvr", cols[2]);
        query.bindValue(":kosgu", cols[3]);
        query.bindValue(":kubp", cols[4]);
        query.bindValue(":notes", cols[5].isEmpty() ? QVariant() : cols[5].simplified().replace("\n", " "));
        query.bindValue(":direction", cols[6].isEmpty() ? QVariant() : cols[6].simplified().replace("\n", " "));
        query.bindValue(":expenses_part", cols[7]);
        query.bindValue(":amount", cols[8]);
        if (!query.exec())
        {
            qCritical() << "Не удалось добавить строку. Строка: " << cols;
            qCritical() << "Детали ошибки: " << query.lastError();
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась. Детали ошибки:\n" + query.lastError().text());
            popUp->show();
            return;
        }
    }
    QSqlDatabase::database().commit();
    dirty = true;
    updateSum();
    mFactModel->select();
}

void EditorFact::on_filter_row_cellChanged(int row, int column)
{
    mFilter = QString("document_id = %1").arg(mIdDoc);
    for (int i = 0; i < ui->filter_row->columnCount(); i++)
    {
        if (ui->filter_row->item(0, i) == nullptr)
        {
            continue;
        }
        if (ui->filter_row->item(0, i)->text().isEmpty())
        {
            continue;
        }
        mFilter += QString(" AND %1 LIKE '%%2%'")
                       .arg(mFactModel->record().fieldName(i), ui->filter_row->item(0, i)->text().replace("'", "\""));
    }
    mFactModel->setFilter(mFilter);
    updateSum();
}

void EditorFact::on_filter_clear_clicked()
{
    for (int i = 0; i < ui->filter_row->columnCount(); i++)
    {
        ui->filter_row->setItem(0, i, nullptr);
    }
    updateSum();
}

void EditorFact::customContextMenuRequested(const QPoint &pos)
{
    QModelIndex idx = ui->fact_table->currentIndex();

    if (idx.isValid())
    {
        QVariant data = ui->fact_table->currentIndex().data(Qt::EditRole);
        ui->filter_row->setItem(0, ui->fact_table->currentIndex().column(), new QTableWidgetItem(data.toString()));
    }
}

void EditorFact::on_action_add_triggered()
{
    QVariant data = ui->fact_table->currentIndex().data(Qt::EditRole);
    ui->filter_row->setItem(0, ui->fact_table->currentIndex().column(), new QTableWidgetItem(data.toString()));
}

void EditorFact::on_action_NULL_triggered()
{
    mFactModel->setData(ui->fact_table->currentIndex(), QVariant());
}

void EditorFact::on_minus_fact_button_clicked()
{
    int res = QMessageBox::question(
        this, "Подтверждение",
        "Данное действие добавит в редактор суммы, обнуляющие финансирование.\nВыполнить операцию?");
    if (res != QMessageBox::Yes)
    {
        return;
    }
    QString filter;
    if (ui->cb_kubp->currentIndex() != -1)
    {
        filter += QString(" AND kubp = '%1'").arg(ui->cb_kubp->currentText());
    }
    if (ui->cb_krp->currentIndex() != -1)
    {
        filter += QString(" AND krp = '%1'").arg(ui->cb_krp->currentText());
    }
    if (ui->cb_kcs->currentIndex() != -1)
    {
        filter += QString(" AND kcs = '%1'").arg(ui->cb_kcs->currentText());
    }
    if (ui->cb_kvr->currentIndex() != -1)
    {
        filter += QString(" AND kvr = '%1'").arg(ui->cb_kvr->currentText());
    }
    if (ui->cb_kosgu->currentIndex() != -1)
    {
        filter += QString(" AND kosgu = '%1'").arg(ui->cb_kosgu->currentText());
    }
    QSqlQuery query;
    query.prepare(Queries::insert_minus_in_exp_fact.arg(filter));
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("При добавлении минусов произошла ошибка. Детали:\n" + query.lastError().text());
        popUp->show();
        qCritical() << query.lastError().text();
        return;
    }
    popUp->setPopupType(Access);
    popUp->setPopupText("Данные успешно добавлены!");
    popUp->show();
    mFactModel->select();
    dirty = true;
    totalsDebounceTimer->start();
}
