#include "editor_plan.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QException>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QShortcut>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QString>
#include <QTableView>
#include <QTimer>
#include <QXmlStreamWriter>

#include "delegate_center_aligment.h"
#include "delegate_expenses_part.h"
#include "delegate_limited_trimmed_text.h"
#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "error_dialog.h"
#include "expenses_part.h"
#include "table_view_utils.h"

struct ColumnData
{
    QString name;
    QString type;
    int width;
};

EditorPlan::EditorPlan(int id, bool read_only, QWidget *parent) : mIdDoc(id), QWidget(parent), ui(new Ui::editor_plan)
{
    ui->setupUi(this);
    setWindowTitle(QString("Справка №%1").arg(getDocNum()));

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    mCurrentYear = settings->value("settings/year").toInt();

    popUp = new PopUp(this);

    dirty = false;
    validation_not_runned = true;
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
            validation_not_runned = true;
        }
    }
    else
    {
        insertToExpensesEditor();
    }

    setDropDownTable(ui->cb_kubp, Queries::kubp_plan);
    setExpensesPart();

    mYearSumModel = new QSqlQueryModel(this);
    ui->row_sum->setModel(mYearSumModel);
    ui->row_sum->setItemDelegateForColumn(0, new DelegateNumberMoney(this));

    mPlanModel = new QSqlTableModel(this);
    mPlanModel->setTable("expenses_editor_plan");
    mPlanModel->setSort(0, Qt::DescendingOrder);
    mPlanModel->setFilter(QString("document_id = %1").arg(mIdDoc));
    mPlanModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    mPlanModel->select();
    ui->plan_table->setModel(mPlanModel);
    ui->plan_table->installEventFilter(this);

    QList<ColumnData> column_data = { { "id", "hidden", 0 },
                                      { "doc_id", "hidden", 0 },
                                      { "КРП", "center_aligment", 39 },
                                      { "КЦС", "center_aligment", 49 },
                                      { "КВР", "center_aligment", 39 },
                                      { "КОСГУ", "center_aligment", 45 },
                                      { "КУБП", "center_aligment", 49 },
                                      { "Январь", "number", 0 },
                                      { "Февраль", "number", 0 },
                                      { "Март", "number", 0 },
                                      { "Апрель", "number", 0 },
                                      { "Май", "number", 0 },
                                      { "Июнь", "number", 0 },
                                      { "Июль", "number", 0 },
                                      { "Август", "number", 0 },
                                      { "Сентябрь", "number", 0 },
                                      { "Октябрь", "number", 0 },
                                      { "Ноябрь", "number", 0 },
                                      { "Декабрь", "number", 0 },
                                      { "Часть расходов", "exp_part", 70 },
                                      { "Примечание", "limited_text", 100 } };
    if (read_only)
    {
        ui->cb_kubp->setEnabled(false);
        ui->cb_notes->setEnabled(false);
        ui->cb_exp_part->setEnabled(false);
        ui->add_kkrb->setEnabled(false);
        ui->delete_row_button->setEnabled(false);
        ui->check_button->setEnabled(false);
        ui->save_button->setEnabled(false);
        ui->plan_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        auto pasteAction = new QAction("Вставить", ui->plan_table);
        pasteAction->setShortcut((QKeySequence(Qt::CTRL + Qt::Key_V)));
        pasteAction->setShortcutContext(Qt::WidgetShortcut);
        connect(pasteAction, &QAction::triggered, this, [=](bool) { pasteFromClipboard(ui->plan_table); });
        ui->plan_table->addAction(pasteAction);
    }
    connect(mPlanModel, &QSqlTableModel::dataChanged, this, &EditorPlan::dataChanged);

    for (int i = 0; i < mPlanModel->columnCount(); i++)
    {
        mPlanModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        ui->column_sum->insertColumn(i);
        ui->filter_row->insertColumn(i);
        QString type = column_data[i].type;
        if (type == "hidden")
        {
            ui->plan_table->hideColumn(i);
            ui->column_sum->hideColumn(i);
            ui->filter_row->hideColumn(i);
        }
        else if (type == "center_aligment")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
            ui->filter_row->setItemDelegateForColumn(i, new DelegateCenterAligment(this));
        }
        else if (type == "limited_text")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(100000, this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(100000, this));
            ui->filter_row->setItemDelegateForColumn(i, new DelegateLimitedTrimmedText(100000, this));
            ui->plan_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            ui->column_sum->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            ui->filter_row->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        }
        else if (type == "number")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItem(0, i, new QTableWidgetItem("0"));
            ui->column_sum->resizeColumnToContents(i);
        }
        else if (type == "exp_part")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
    }

    on_filter_row_cellChanged(0, 0);

    TableViewUtils::applyStyle(ui->plan_table);

    totalsDebounceTimer = new QTimer(this);
    totalsDebounceTimer->setInterval(400);
    totalsDebounceTimer->setSingleShot(true);
    connect(totalsDebounceTimer, &QTimer::timeout, this, &EditorPlan::updateSum);
    totalsDebounceTimer->start();

    ui->row_sum->setAlternatingRowColors(true);
    ui->row_sum->horizontalHeader()->setVisible(true);
    ui->plan_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(ui->plan_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->column_sum->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->column_sum->horizontalScrollBar(), &QScrollBar::valueChanged, ui->plan_table->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->plan_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->filter_row->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->filter_row->horizontalScrollBar(), &QScrollBar::valueChanged, ui->plan_table->horizontalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->plan_table->verticalScrollBar(), &QScrollBar::valueChanged, ui->row_sum->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->row_sum->verticalScrollBar(), &QScrollBar::valueChanged, ui->plan_table->verticalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->plan_table->horizontalHeader(), &QHeaderView::sectionResized, this,
            [this](int logicalIndex) { resizeColumnSumWidth(logicalIndex); });

    resizeRowHeight();

    rightClickMenu = new QMenu(this);
    ui->plan_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->plan_table, &QTableView::customContextMenuRequested, this, &EditorPlan::customContextMenuRequested);

    QApplication::restoreOverrideCursor();
}

EditorPlan::~EditorPlan()
{
    delete ui;
}

int EditorPlan::getDocNum() const
{
    QSqlQuery query;
    query.prepare(Queries::num_from_plan_doc);
    query.bindValue(":id", mIdDoc);
    query.exec();
    if (query.next())
    {
        return query.value("doc_number").toInt();
    }
    return 0;
}

void EditorPlan::insertToExpensesEditor()
{
    QSqlQuery query;
    query.prepare(Queries::delete_in_exp_plan);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Ошибка выполнения запроса. Не удалось очистить редактор!");
        popUp->show();
        qCritical() << query.lastError();
        return;
    }
    query.prepare(Queries::insert_in_exp_plan);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Ошибка выполнения запроса. Не удалось добавить данные в редактор!");
        popUp->show();
        qCritical() << query.lastError();
        return;
    }
}

void EditorPlan::setDropDownTable(QComboBox *rComboBox, const QString &rQuery)
{
    QSqlDatabase sdb = QSqlDatabase::database();
    QSqlQuery query;
    query.prepare(rQuery);
    query.bindValue(":document_id", mIdDoc);
    query.bindValue(":kubp", ui->cb_kubp->currentText());
    query.bindValue(":krp", ui->cb_krp->currentText());
    query.bindValue(":kcs", ui->cb_kcs->currentText());
    query.bindValue(":kvr", ui->cb_kvr->currentText());
    query.exec();

    QSqlQueryModel *model = new ModelKKRB(this);
    model->setQuery(query);
    model->setHeaderData(0, Qt::Horizontal, "Код");
    model->setHeaderData(1, Qt::Horizontal, "Наименование");

    QTableView *table_view = new QTableView(this);
    rComboBox->setView(table_view);
    rComboBox->setModel(model);

    // скрываем колонку "checker"
    table_view->hideColumn(2);

    // многострочный текст
    //    table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // скрыть номера строк
    table_view->verticalHeader()->hide();

    // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
    table_view->setMinimumWidth(mWidthDropDown);
    for (int i = 0; i < model->rowCount(); i++)
    {
        table_view->setRowHeight(i, 11);
    }
    table_view->setColumnWidth(0, 100);
    table_view->setColumnWidth(1, mWidthDropDown - table_view->columnWidth(0) - 17);

    // выбор всей строки, а не ячейки
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    rComboBox->setModelColumn(0);
    rComboBox->setCurrentIndex(-1);
}

void EditorPlan::resizeColWidth()
{
    for (int i = 2; i < mPlanModel->columnCount(); i++)
    {
        ui->column_sum->resizeColumnToContents(i);
        ui->plan_table->resizeColumnToContents(i);
        int width = ui->column_sum->columnWidth(i) > ui->plan_table->columnWidth(i) ?
                        ui->column_sum->columnWidth(i) :
                        ui->plan_table->columnWidth(i) + 10;
        QTableWidgetItem *item = ui->column_sum->item(0, i);
        if (item != nullptr)
        {
            width += 15;
        }
        ui->column_sum->setColumnWidth(i, width);
        ui->filter_row->setColumnWidth(i, width);
        ui->plan_table->setColumnWidth(i, width);
    }
}

void EditorPlan::resizeColumnSumWidth(int ind)
{
    ui->column_sum->setColumnWidth(ind, ui->plan_table->columnWidth(ind));
    ui->filter_row->setColumnWidth(ind, ui->plan_table->columnWidth(ind));
}

void EditorPlan::resizeRowHeight()
{
    ui->peg->setMaximumWidth(ui->plan_table->verticalHeader()->width());
    ui->peg_2->setMaximumWidth(ui->plan_table->verticalHeader()->width());
}

void EditorPlan::setExpensesPart()
{
    ui->cb_exp_part->clear();
    auto i = ExpensesPart::names.constBegin();
    while (i != ExpensesPart::names.constEnd())
    {
        ui->cb_exp_part->insertItem(-1, i.value());
        ++i;
    }
    ui->cb_exp_part->setCurrentIndex(-1);
}

QString EditorPlan::getExpensesPart() const
{
    return ExpensesPart::names.key(ui->cb_exp_part->currentText());
}

bool EditorPlan::checkIdDoc() const
{
    QSqlQuery query;
    query.prepare(Queries::check_id_plan);
    query.bindValue(":id", mIdDoc);
    query.exec();
    if (query.next())
    {
        if (query.value(0).toInt() > 0)
        {
            return true;
        }
    }
    return false;
}

QSqlError EditorPlan::saveData()
{
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    query.prepare(Queries::delete_in_plan_data);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        QSqlDatabase::database().rollback();
        return query.lastError();
    }
    query.prepare(Queries::insert_in_plan_data);
    query.bindValue(":id", mIdDoc);
    query.bindValue(":year", mCurrentYear);
    if (!query.exec())
    {
        QSqlDatabase::database().rollback();
        return query.lastError();
    }
    QSqlDatabase::database().commit();
    return query.lastError();
}

bool EditorPlan::checkDuplicate()
{
    on_filter_clear_clicked();
    QSqlQuery query;
    query.prepare(Queries::check_duplicate_exp_plan);
    query.bindValue(":id", mIdDoc);
    query.exec();
    if (query.next())
    {
        int j = 0;
        for (int i = 0; i < ui->filter_row->columnCount(); i++)
        {
            if (ui->filter_row->isColumnHidden(i) || (i >= 7 && i <= 18))
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

void EditorPlan::pasteFromClipboard(QTableView *view, Qt::ItemDataRole role)
{
    popUp->setPopupType(Warning);
    QStringList rows = QApplication::clipboard()->text().split('\n', Qt::SkipEmptyParts);

    const QList<QString> exp_part_names_values = ExpensesPart::names.values();
    QSqlDatabase::database().transaction();
    for (int i = 0; i < rows.size(); i++)
    {
        QStringList cols = rows.at(i).split('\t');
        if (cols.size() != 18 && cols.size() != 19)
        {
            QString error_str =
                QString(
                    "При вставке не то количество столбиков. Должно быть 18 или 19, а получили %1. Данные "
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
        for (size_t i = 5; i < 17; i++)
        {
            cols[i] = cols[i].replace(" ", "");
            if (!re_double.exactMatch(cols[i]))
            {
                QString error_str =
                    QString("Сумма в %1 месяце должна быть числом, а было получено %2").arg(i - 4).arg(cols[i]);
                qCritical() << error_str << cols;
                QSqlDatabase::database().rollback();
                popUp->setPopupText("Вставка не удалась.\n" + error_str);
                popUp->show();
                return;
            }
        }
        if (!exp_part_names_values.contains(cols[17]))
        {
            QString error_str = QString("Часть расходов не верная (%1 нет в базе)").arg(cols[17]);
            qCritical() << error_str << cols;
            QSqlDatabase::database().rollback();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            break;
        }
        else
        {
            cols[17] = ExpensesPart::names.key(cols[17]);
        }
        QVariant notes;
        if (cols.size() == 19)
        {
            if (!cols[18].isEmpty())
            {
                notes = cols[18].simplified().replace("\n", " ");
            }
        }
        QSqlQuery query;
        query.prepare(Queries::insert_in_exp_plan_manual);
        query.bindValue(":id", mIdDoc);
        query.bindValue(":krp", cols[0]);
        query.bindValue(":kcs", cols[1]);
        query.bindValue(":kvr", cols[2]);
        query.bindValue(":kosgu", cols[3]);
        query.bindValue(":kubp", cols[4]);
        query.bindValue(":m_1", cols[5]);
        query.bindValue(":m_2", cols[6]);
        query.bindValue(":m_3", cols[7]);
        query.bindValue(":m_4", cols[8]);
        query.bindValue(":m_5", cols[9]);
        query.bindValue(":m_6", cols[10]);
        query.bindValue(":m_7", cols[11]);
        query.bindValue(":m_8", cols[12]);
        query.bindValue(":m_9", cols[13]);
        query.bindValue(":m_10", cols[14]);
        query.bindValue(":m_11", cols[15]);
        query.bindValue(":m_12", cols[16]);
        query.bindValue(":expenses_part", cols[17]);
        query.bindValue(":notes", notes);
        if (!query.exec())
        {
            QString error_str =
                QString("Не удалось добавить строку в базу данных. Детали ошибки: " + query.lastError().text());
            qCritical() << "Не удалось добавить строку. Строка: " << cols;
            qCritical() << "Детали ошибки: " << query.lastError();
            popUp->setPopupText("Вставка не удалась.\n" + error_str);
            popUp->show();
            return;
        }
    }
    QSqlDatabase::database().commit();
    dirty = true;
    updateSum();
    mPlanModel->select();
}

void EditorPlan::updateSum()
{
    if (mPlanModel->lastError().type() != QSqlError::NoError)
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Неверный ввод. Детали ошибки:\n" + mPlanModel->lastError().text());
        popUp->show();
    }
    QSqlQuery query_sum;
    query_sum.prepare(Queries::sum_row_exp_plan.arg(mFilter));
    query_sum.bindValue(":id", mIdDoc);
    query_sum.exec();
    mYearSumModel->setQuery(query_sum);
    mYearSumModel->setHeaderData(0, Qt::Horizontal, "[Год]");
    QSqlQuery query;
    query.prepare(Queries::sum_col_exp_plan.arg(mFilter));
    query.exec();
    double sum_total = 0;
    if (query.next())
    {
        for (int i = 0; i < 12; i++)
        {
            double sum = query.value(i).toDouble();
            sum_total += sum;
            ui->column_sum->item(0, i + 7)->setText(getMoneyNumber(sum));
        }
    }
    ui->year_sum->setText(getMoneyNumber(sum_total));
    ui->year_sum->setMaximumWidth(ui->row_sum->horizontalHeader()->width());
    resizeColWidth();
}

bool EditorPlan::checkCode() const
{
    QSqlQuery query;
    query.prepare(Queries::check_kkrb_exp_plan);
    query.bindValue(":id", mIdDoc);
    query.bindValue(":krp", ui->cb_krp->currentText());
    query.bindValue(":kcs", ui->cb_kcs->currentText());
    query.bindValue(":kvr", ui->cb_kvr->currentText());
    query.bindValue(":kosgu", ui->cb_kosgu->currentText());
    query.bindValue(":kubp", ui->cb_kubp->currentText());
    query.bindValue(":exp_part", getExpensesPart());
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

bool EditorPlan::checkKosgu() const
{
    QSqlQuery query;
    query.prepare(Queries::check_kosgu_with_notes);
    query.bindValue(":code", ui->cb_kosgu->currentText());
    query.exec();
    while (query.next())
    {
        if (query.value(0).toBool() && ui->cb_notes->currentText().isEmpty())
        {
            return true;
        }
    }
    return false;
}

void EditorPlan::on_cb_kubp_currentIndexChanged(int index)
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
    if (index != -1)
    {
        setDropDownTable(ui->cb_krp, Queries::kkrb_krp_plan);
        ui->cb_krp->setEnabled(true);
        ui->cb_krp->setFocus();
    }
}

void EditorPlan::on_cb_krp_currentIndexChanged(int index)
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
        if (index != -1)
        {
            setDropDownTable(ui->cb_kcs, Queries::kkrb_kcs_plan);
            ui->cb_kcs->setEnabled(true);
            ui->cb_kcs->setFocus();
        }
    }
}

void EditorPlan::on_cb_kcs_currentIndexChanged(int index)
{
    ui->cb_kcs->lineEdit()->setStyleSheet("");
    if (ui->cb_kcs->isEnabled())
    {
        ui->cb_kvr->setEnabled(false);
        ui->cb_kosgu->setEnabled(false);
        ui->cb_kvr->setCurrentIndex(-1);
        ui->cb_kosgu->setCurrentIndex(-1);
        if (index != -1)
        {
            setDropDownTable(ui->cb_kvr, Queries::kkrb_kvr_plan);
            ui->cb_kvr->setEnabled(true);
            ui->cb_kvr->setFocus();
        }
    }
}

void EditorPlan::on_cb_kvr_currentIndexChanged(int index)
{
    ui->cb_kvr->lineEdit()->setStyleSheet("");
    if (ui->cb_kvr->isEnabled())
    {
        if (index == -1)
        {
            ui->cb_kosgu->setEnabled(false);
            ui->cb_kosgu->setCurrentIndex(-1);
        }
        else
        {
            setDropDownTable(ui->cb_kosgu, Queries::kkrb_kosgu_plan);
            ui->cb_kosgu->setEnabled(true);
            ui->cb_kosgu->setFocus();
        }
    }
}

void EditorPlan::on_cb_kosgu_currentIndexChanged(int index)
{
    ui->cb_kosgu->lineEdit()->setStyleSheet("");
    ui->add_kkrb->setFocus();
    if (ui->cb_kosgu->isEnabled())
    {
        if (index == -1)
        {
            ui->cb_notes->setEnabled(false);
            ui->cb_notes->setCurrentIndex(-1);
        }
        else
        {
            // NOTES
            ui->cb_notes->setEnabled(true);
            QSqlQuery query;
            query.prepare(Queries::notes_plan);
            query.bindValue(":document_id", mIdDoc);
            query.bindValue(":kubp", ui->cb_kubp->currentText());
            query.bindValue(":krp", ui->cb_krp->currentText());
            query.bindValue(":kcs", ui->cb_kcs->currentText());
            query.bindValue(":kvr", ui->cb_kvr->currentText());
            query.bindValue(":kosgu", ui->cb_kosgu->currentText());
            query.exec();

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery(query);

            QTableView *table_view = new QTableView(this);
            table_view->horizontalHeader()->hide();
            table_view->verticalHeader()->hide();
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
        }
    }
}

void EditorPlan::on_add_kkrb_clicked()
{
    if (checkCode())
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Такой ККРБ уже есть в справке!");
        popUp->show();
        return;
    }
    if (checkKosgu())
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText(QString("У КОСГУ '%1' должен быть с примечанием!").arg(ui->cb_kosgu->currentText()));
        popUp->show();
        return;
    }
    ui->cb_kubp->lineEdit()->setStyleSheet("");
    ui->cb_krp->lineEdit()->setStyleSheet("");
    ui->cb_kcs->lineEdit()->setStyleSheet("");
    ui->cb_kvr->lineEdit()->setStyleSheet("");
    ui->cb_kosgu->lineEdit()->setStyleSheet("");
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
    if (ui->cb_kubp->findText(ui->cb_kubp->currentText()) == -1)
    {
        ui->cb_kubp->lineEdit()->setStyleSheet("border: 1px solid red");
        return;
    }
    if (ui->cb_exp_part->currentIndex() == -1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите часть расходов!");
        popUp->show();
        return;
    }

    QSqlRecord record = mPlanModel->record();
    record.setValue("document_id", mIdDoc);
    record.setValue("krp", ui->cb_krp->currentText());
    record.setValue("kcs", ui->cb_kcs->currentText());
    record.setValue("kvr", ui->cb_kvr->currentText());
    record.setValue("kosgu", ui->cb_kosgu->currentText());
    record.setValue("kubp", ui->cb_kubp->currentText());
    record.setValue("notes", ui->cb_notes->currentText().isEmpty() ? QVariant() : ui->cb_notes->currentText());
    record.setValue("expenses_part", getExpensesPart());
    record.setGenerated("id", false);
    for (int i = record.indexOf("m_1"); i <= record.indexOf("m_12"); i++)
    {
        record.setGenerated(i, false);
    }
    qDebug() << record;
    if (!mPlanModel->insertRecord(0, record))
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Ошибка добавления ККРБ. Не удалось добавить ККРБ в базу данных.");
        popUp->show();
        qCritical() << record << mPlanModel->lastError();
        return;
    }

    if (ui->check_clear_kkrb->isChecked())
    {
        ui->cb_kubp->setCurrentIndex(-1);
        ui->cb_notes->setCurrentIndex(-1);
        ui->cb_exp_part->setCurrentIndex(-1);
    }
    ui->plan_table->setCurrentIndex(mPlanModel->index(0, 7));
    ui->plan_table->setFocus();
}

void EditorPlan::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    dirty = true;
    validation_not_runned = true;
    totalsDebounceTimer->start();
}

void EditorPlan::closeEvent(QCloseEvent *event)
{
    updateSum();
    if (dirty)
    {
        int res =
            QMessageBox::question(this, "Сохранение данных", "Обнаружены не сохраненные данные, хотите их сохранить?",
                                  QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard, QMessageBox::Save);

        if (res == QMessageBox::Save)
        {
            if (validation_not_runned)
            {
                on_check_button_clicked();
                if (validation_not_runned)
                {
                    event->ignore();
                    return;
                }
            }
            QSqlError er = saveData();
            if (er.type() != QSqlError::NoError)
            {
                QMessageBox::critical(this, "Ошибка",
                                      "При сохранении данных произошла ошибка. Детали:\n" + er.driverText());
                qCritical() << er.driverText();
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
    QSqlQuery query;
    query.prepare(Queries::delete_in_exp_plan);
    query.bindValue(":id", mIdDoc);
    if (!query.exec())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("При очистке редактора произошла ошибка. Детали:\n" + query.lastError().driverText());
        popUp->show();
        qCritical() << query.lastError().driverText();
        event->ignore();
        return;
    }
}

void EditorPlan::on_delete_row_button_clicked()
{
    QSet<int> set;
    const auto selected_index = ui->plan_table->selectionModel()->selectedIndexes();
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
            if (!mPlanModel->removeRow(ind_row))
            {
                popUp->setPopupType(Error);
                popUp->setPopupText(QString("Ошибка при удалении строки №%1 из базы данных.").arg(ind_row));
                popUp->show();
                QSqlDatabase::database().rollback();
                qCritical() << mPlanModel->lastError();
                return;
            }
            dirty = true;
            validation_not_runned = true;
        }
        QSqlDatabase::database().commit();
        updateSum();
        mPlanModel->select();
    }
}

void EditorPlan::on_check_button_clicked()
{
    if (!checkDuplicate())
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Найдены дубликаты. Проверьте данные!");
        popUp->show();
        return;
    }
    popUp->setPopupType(Access);
    popUp->setPopupText("Проверка успешно пройдена!");

    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ErrorDialog er;
    validation_not_runned = !er.check_plan(mIdDoc);

    QApplication::restoreOverrideCursor();
    QWidget::setEnabled(true);

    if (validation_not_runned)
    {
        er.setModal(true);
        er.exec();
        QCoreApplication::processEvents();
    }
    else
    {
        popUp->show();
    }
}

void EditorPlan::on_save_button_clicked()
{
    if (validation_not_runned)
    {
        on_check_button_clicked();
        if (validation_not_runned)
        {
            return;
        }
    }
    QSqlError er = saveData();
    if (er.type() != QSqlError::NoError)
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("При сохранении данных произошла ошибка. Детали:\n" + er.driverText());
        popUp->show();
        qCritical() << er.driverText();
        return;
    }
    dirty = false;
    popUp->setPopupType(Access);
    popUp->setPopupText("Данные успешно сохранены!");
    popUp->show();
}

void EditorPlan::on_minus_plan_button_clicked()
{
    int res = QMessageBox::question(this, "Подтверждение",
                                    "Данное действие добавит в редактор суммы, обнуляющие план.\nВыполнить операцию?");
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
    query.prepare(Queries::insert_minus_in_exp_plan.arg(filter));
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
    mPlanModel->select();
    dirty = true;
    validation_not_runned = true;
    totalsDebounceTimer->start();
}

void EditorPlan::on_filter_clear_clicked()
{
    for (int i = 0; i < ui->filter_row->columnCount(); i++)
    {
        ui->filter_row->setItem(0, i, nullptr);
    }
    updateSum();
}

void EditorPlan::on_filter_row_cellChanged(int row, int column)
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
                       .arg(mPlanModel->record().fieldName(i), ui->filter_row->item(0, i)->text().replace("'", "\""));
    }
    mPlanModel->setFilter(mFilter);
    updateSum();
}

void EditorPlan::customContextMenuRequested(const QPoint &pos)
{
    QModelIndex idx = ui->plan_table->currentIndex();
    rightClickMenu->clear();

    if (idx.isValid())
    {
        QVariant data = idx.data(Qt::EditRole);
        ui->filter_row->setItem(0, idx.column(), new QTableWidgetItem(data.toString()));
    }
}

void EditorPlan::on_action_NULL_triggered()
{
    mPlanModel->setData(ui->plan_table->currentIndex(), QVariant());
}

void EditorPlan::on_action_add_triggered()
{
    QVariant data = ui->plan_table->currentIndex().data(Qt::EditRole);
    ui->filter_row->setItem(0, ui->plan_table->currentIndex().column(), new QTableWidgetItem(data.toString()));
}

void EditorPlan::on_minus_ost_button_clicked()
{
    int res = QMessageBox::question(
        this, "Подтверждение", "Данное действие добавит в редактор суммы, обнуляющие остатки.\nВыполнить операцию?");
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
    query.prepare(Queries::insert_minus_ost_in_exp_plan.arg(filter));
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
    mPlanModel->select();
    dirty = true;
    validation_not_runned = true;
    totalsDebounceTimer->start();
}
