#include "analitic_plan.h"

#include <QAction>
#include <QDate>
#include <QException>
#include <QScrollBar>
#include <QSqlQuery>
#include <QTimer>

#include "delegate_expenses_part.h"
#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "expenses_part.h"
#include "sql_queries.h"
#include "table_view_utils.h"
#include "ui_analitic_plan.h"

AnaliticPlan::AnaliticPlan(Type type, QWidget *parent) : QWidget(parent), ui(new Ui::AnaliticPlan)
{
    ui->setupUi(this);
    mModel = new QSqlTableModel(this);

    ui->time_groupBox->setVisible(false);

    mType = type;

    switch (mType)
    {
    case PlanData:
        setWindowTitle("Планы с уточнениями");
        mModel->setTable("analitic_plan_data");
        column_data = { { "Месяц", "read_only", 50 },       { "КГРБС", "read_only", 39 },
                        { "КРП", "read_only", 39 },         { "КЦС", "read_only", 49 },
                        { "КВР", "read_only", 39 },         { "КОСГУ", "read_only", 45 },
                        { "КУБП", "read_only", 49 },        { "Часть расходов", "exp_part", 70 },
                        { "Примечание", "read_only", 400 }, { "Сумма", "number", 0 } };
        break;
    case PlanPivotData:
        setWindowTitle("Планы с уточнениями (помесячно)");
        mModel->setTable("pivot_plan_data_without_document");
        column_data = {
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "[ Год ]", "number", 0 },
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
        };
        break;
    case PlanPivotDataDocument:
        setWindowTitle("Справки помесячно");
        mModel->setTable("pivot_plan_data_with_document");
        column_data = {
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "Номер справки", "read_only", 80 },
            { "Дата справки", "read_only", 80 },
            { "Основание", "read_only", 80 },
            { "[ Год ]", "number", 0 },
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
        };
        break;
    case FinanceDataDocumentMonth:
        setWindowTitle("Финансирование помесячно");
        mModel->setTable("analitic_finance_month");
        column_data = {
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "[ Год ]", "number", 0 },
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
        };
        break;
    case FinanceDataDocumentYear:
        setWindowTitle("Финансирование за год");
        mModel->setTable("analitic_finance");
        column_data = { { "КГРБС", "read_only", 39 },
                        { "КРП", "read_only", 39 },
                        { "КЦС", "read_only", 49 },
                        { "КВР", "read_only", 39 },
                        { "КОСГУ", "read_only", 45 },
                        { "КУБП", "read_only", 49 },
                        { "Часть расходов", "exp_part", 70 },
                        { "Примечание", "read_only", 400 },
                        { "Номер документа", "read_only", 80 },
                        { "Дата документа", "read_only", 80 },
                        { "", "read_only", 80 },
                        { "Сумма", "number", 0 } };
        break;
    case RemnantsIncrementallyYear:
        setWindowTitle("Остаток по нарастающим за год");
        mModel->setTable("remnants_incrementally");
        column_data = {
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "План", "number", 0 },
            { "Факт", "number", 0 },
            { "Остаток", "number", 0 },
        };
        break;
    case RemnantsIncrementallyMonth:
        setWindowTitle("Остаток по нарастающим помесячно");
        mModel->setTable("remnants_incrementally_year");
        column_data = {
            { "Месяц", "read_only", 50 },
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "План", "number", 0 },
            { "Факт", "number", 0 },
            { "Остаток", "number", 0 },
        };
        break;
    case RemnantsIncrementallyPeriod:
        setWindowTitle("Остаток по нарастающим за период");
        ui->data_table->installEventFilter(this);
        ui->time_groupBox->setVisible(true);

        QDate d_start = QDate(QDate::currentDate().year(), 1, 1);
        QDate d_end = QDate::currentDate();

        ui->date_start->setDate(d_start);
        ui->date_end->setDate(d_end);

        QSqlQuery query;
        query.exec(Queries::drop_remnants_incrementally_period);
        query.exec(Queries::create_remnants_incrementally_period.arg(d_start.month())
                       .arg(d_end.month())
                       .arg(d_start.toString("yyyy-MM-dd"), d_end.toString("yyyy-MM-dd")));
        mModel->setTable("remnants_incrementally_period");
        column_data = {
            { "КГРБС", "read_only", 39 },
            { "КРП", "read_only", 39 },
            { "КЦС", "read_only", 49 },
            { "КВР", "read_only", 39 },
            { "КОСГУ", "read_only", 45 },
            { "КУБП", "read_only", 49 },
            { "Часть расходов", "exp_part", 70 },
            { "Примечание", "read_only", 400 },
            { "План", "number", 0 },
            { "Факт", "number", 0 },
            { "Остаток", "number", 0 },
        };
        break;
    }

    mModel->select();
    ui->data_table->setModel(mModel);
    ui->data_table->resizeColumnsToContents();

    for (int i = 0, j = 0; i < mModel->columnCount(); i++)
    {
        mModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        ui->column_sum->insertColumn(i);
        QString type = column_data[i].type;
        if (type == "hidden")
        {
            ui->data_table->hideColumn(i);
            ui->column_sum->hideColumn(i);
        }
        else if (type == "read_only")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateReadOnly(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateReadOnly(this));
        }
        else if (type == "number")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItem(0, i, new QTableWidgetItem("0"));
        }
        else if (type == "exp_part")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
    }

    resizeColSum();

    ui->filter_widget->setModel(mModel);

    TableViewUtils::applyStyle(ui->data_table);

    filterDebounceTimer = new QTimer(this);
    filterDebounceTimer->setInterval(800);
    filterDebounceTimer->setSingleShot(true);
    connect(filterDebounceTimer, &QTimer::timeout, this, &AnaliticPlan::updateFilter);
    connect(ui->filter_widget, &SqlExtendedFilter::filterChanged, this,
            [this](const QString &sql) { filterDebounceTimer->start(); });

    ui->filter_widget->setFieldsByHeader(ui->data_table->horizontalHeader());

    connect(ui->data_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->column_sum->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->column_sum->horizontalScrollBar(), &QScrollBar::valueChanged, ui->data_table->horizontalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->data_table->horizontalHeader(), &QHeaderView::sectionResized, this,
            [this](int logicalIndex) { resizeTableCol(logicalIndex); });

    rightClickMenu = new QMenu(this);
    ui->data_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->data_table, &QTableView::customContextMenuRequested, this, &AnaliticPlan::customContextMenuRequested);
}

void AnaliticPlan::resizeColSum()
{
    for (int i = 0; i < mModel->columnCount(); i++)
    {
        //!!!!!!!!!!!!!!!!!!!!!!!! Обрабатывать только номера !!!!!!!!!!!!!!!!!!!!!
        if (column_data[i].type == "number")
        {
            ui->data_table->resizeColumnToContents(i);
            ui->column_sum->resizeColumnToContents(i);
            int col_width = ui->column_sum->columnWidth(i);
            int table_width = ui->data_table->columnWidth(i);
            int max_width = col_width < table_width ? table_width : col_width;
            ui->data_table->setColumnWidth(i, max_width);
            ui->column_sum->setColumnWidth(i, max_width);
        }
        else
        {
            ui->data_table->resizeColumnToContents(i);
            ui->column_sum->setColumnWidth(i, ui->data_table->columnWidth(i));
        }
    }
}

void AnaliticPlan::resizeTableCol(const int &ind)
{
    ui->column_sum->setColumnWidth(ind, ui->data_table->columnWidth(ind));
}

void AnaliticPlan::updateFilter()
{
    ui->column_sum->clear();
    QList<double> col_sum;
    QSqlQuery query;
    QString sql_text;
    QString sql = ui->filter_widget->filter();
    switch (mType)
    {
    case Type::PlanData:
        sql_text = Queries::sum_analitic_plan_data;
        break;
    case Type::PlanPivotData:
        sql_text = Queries::sum_analitic_plan_data_pivot;
        break;
    case Type::PlanPivotDataDocument:
        sql_text = Queries::sum_analitic_plan_data_document_pivot;
        break;
    case Type::FinanceDataDocumentMonth:
        sql_text = Queries::sum_analitic_finance_month;
        break;
    case Type::FinanceDataDocumentYear:
        sql_text = Queries::sum_analitic_finance_year;
        break;
    case Type::RemnantsIncrementallyYear:
        sql_text = Queries::sum_remnants_incrementally;
        break;
    case Type::RemnantsIncrementallyMonth:
        sql_text = Queries::sum_remnants_incrementally_year;
        break;
    case Type::RemnantsIncrementallyPeriod:
        sql_text = Queries::sum_remnants_incrementally_period;
        break;
    }
    if (sql != "")
    {
        sql_text += " WHERE " + sql;
    }
    mModel->setFilter(sql);
    query.exec(sql_text);
    while (query.next())
    {
        for (int i = 0; i < query.record().count(); i++)
        {
            col_sum.append(query.value(i).toDouble());
        }
    }
    for (int i = 0, j = 0; i < mModel->columnCount(); i++)
    {
        if (column_data[i].type == "number")
        {
            ui->column_sum->setItem(0, i, new QTableWidgetItem(getMoneyNumber(col_sum[j])));
            j++;
        }
    }
    resizeColSum();
}

AnaliticPlan::~AnaliticPlan()
{
    delete ui;
}

void AnaliticPlan::customContextMenuRequested(const QPoint &pos)
{
    QVariant data = ui->data_table->currentIndex().data(Qt::EditRole);
    QModelIndex idx = ui->data_table->currentIndex();
    rightClickMenu->clear();
    const int maxlen = 40;
    QString dataString = data.toString();
    if (dataString.length() > maxlen)
    {
        dataString = dataString.left(maxlen - 3) + "...";
    }

    if (idx.isValid())
    {
        QSqlField field = mModel->record().field(idx.column());
        if (field.name() == "expenses_part")
        {
            add_exp_part_in_menu();
            rightClickMenu->popup(ui->data_table->viewport()->mapToGlobal(pos));
            return;
        }
        rightClickMenu->addAction(ui->action_add);
        rightClickMenu->popup(ui->data_table->viewport()->mapToGlobal(pos));
    }
}

void AnaliticPlan::on_action_add_triggered()
{
    QModelIndex idx = ui->data_table->currentIndex();
    if (idx.isValid())
    {
        QVariant data = idx.data(Qt::EditRole);
        QSqlField field = mModel->record().field(idx.column());
        if (ui->filter_widget->filter().isEmpty())
        {
            ui->filter_widget->clear();
        }
        ui->filter_widget->addRule(SqlExtendedFilterNode::WithoutInversion, SqlExtendedFilterNode::ActionEquals, data,
                                   field);
    }
}

void AnaliticPlan::on_action_add_exp_part(const QString &exp_part)
{
    QString data = ExpensesPart::names.key(exp_part);
    if (ui->filter_widget->filter().isEmpty())
    {
        ui->filter_widget->clear();
    }
    ui->filter_widget->addRule(SqlExtendedFilterNode::WithoutInversion, SqlExtendedFilterNode::ActionEquals, data,
                               mModel->record().field("exp_part"));
}

void AnaliticPlan::closeEvent(QCloseEvent *event)
{
    QSqlQuery query;
    query.exec(Queries::drop_remnants_incrementally_period);
}

void AnaliticPlan::add_exp_part_in_menu()
{
    QMenu *exp_part = rightClickMenu->addMenu("Выбор части расходов");
    QMap<QString, QString>::const_iterator i = ExpensesPart::names.constBegin();
    while (i != ExpensesPart::names.constEnd())
    {
        QAction *action = exp_part->addAction(i.value());
        connect(action, &QAction::triggered, this, [=]() { AnaliticPlan::on_action_add_exp_part(action->text()); });
        ++i;
    }
}

void AnaliticPlan::on_accept_period_clicked()
{
    QSqlQuery query;
    query.exec(Queries::drop_remnants_incrementally_period);
    query.exec(Queries::create_remnants_incrementally_period.arg(ui->date_start->date().month())
                   .arg(ui->date_end->date().month())
                   .arg(ui->date_start->date().toString("yyyy-MM-dd"), ui->date_end->date().toString("yyyy-MM-dd")));
    mModel->select();
    updateFilter();
}
