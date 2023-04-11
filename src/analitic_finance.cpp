#include "analitic_finance.h"

#include <QScrollBar>
#include <QSqlQuery>
#include <QTimer>

#include "delegate_expenses_part.h"
#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "expenses_part.h"
#include "sql_queries.h"
#include "table_view_utils.h"
#include "ui_analitic_finance.h"

AnaliticFinance::AnaliticFinance(QWidget *parent) : QWidget(parent), ui(new Ui::AnaliticFinance)
{
    ui->setupUi(this);
    mModel = new QSqlTableModel(this);
    mIncModel = new QSqlQueryModel(this);

    QList<double> col_sum;
    QSqlQuery query;
    setWindowTitle("Остаток по нарастающим помесячно");
    mModel->setTable("remnants_year");
    mColumnNames = { { "Месяц", "read_only", 50 },
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
                     { "Остаток", "number", 0 } };
    query.exec(Queries::sum_remnants_year);
    while (query.next())
    {
        for (int i = 0; i < query.record().count(); i++)
        {
            col_sum.append(query.value(i).toDouble());
        }
    }
    mModel->select();
    ui->data_table->setModel(mModel);
    ui->data_table->resizeColumnsToContents();

    for (int i = 0, j = 0; i < mModel->columnCount(); i++)
    {
        mModel->setHeaderData(i, Qt::Horizontal, mColumnNames[i].name);
        ui->column_sum->insertColumn(i);
        ui->column_sum->setColumnWidth(i, ui->data_table->columnWidth(i));  // сразу работаем с нижней строкой
        QString type = mColumnNames[i].type;
        if (type == "hidden")
        {
            ui->data_table->hideColumn(i);
            ui->column_sum->hideColumn(i);
        }
        else if (type == "read_only")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateReadOnly(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateReadOnly(this));
            if (mColumnNames[i].name == "Примечание")
            {
                ui->data_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
                ui->column_sum->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            }
        }
        else if (type == "exp_part")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
        else if (type == "number")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            int frac = static_cast<long long int>((col_sum[j] * 100)) % 100;
            QString num = QString::number(col_sum[j], 'f', (frac) ? 2 : 0);
            int shift = (col_sum[j] < 0) ? 1 : 0;
            for (int i = num.size() - ((frac) ? 6 : 3); i >= 1 + shift; i -= 3)
            {
                num.insert(i, ' ');
            }
            ui->column_sum->setItem(0, i, new QTableWidgetItem(num));
            ui->column_sum->resizeColumnToContents(i);
            j++;
        }
    }

    mIncModel->setQuery(Queries::remnants_incrementally.arg(""));
    mIncModel->setHeaderData(0, Qt::Horizontal, tr("План"));
    mIncModel->setHeaderData(1, Qt::Horizontal, tr("Факт"));
    mIncModel->setHeaderData(2, Qt::Horizontal, tr("Остаток"));
    ui->incrementally->setModel(mIncModel);
    for (int i = 0; i < 3; i++)
    {
        ui->incrementally->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
        ui->incrementally->setColumnWidth(i, 100);
    }

    ui->data_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->column_sum->setSelectionBehavior(QAbstractItemView::SelectRows);
    resizeColSum();

    TableViewUtils::applyStyle(ui->data_table);
    TableViewUtils::applyStyle(ui->incrementally);

    ui->filter_widget->setModel(mModel);
    ui->filter_widget->setPrependTableNameTo({ "" });

    filterDebounceTimer = new QTimer(this);
    filterDebounceTimer->setInterval(800);
    filterDebounceTimer->setSingleShot(true);
    connect(filterDebounceTimer, &QTimer::timeout, this, &AnaliticFinance::updateFilter);
    connect(ui->filter_widget, &SqlExtendedFilter::filterChanged, this,
            [this](const QString &sql) { filterDebounceTimer->start(); });

    ui->filter_widget->setFieldsByHeader(ui->data_table->horizontalHeader());
    ui->filter_widget->setHiddenFields({ "plan", "fact", "ost" });

    connect(ui->data_table->horizontalScrollBar(), &QScrollBar::valueChanged, ui->column_sum->horizontalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->column_sum->horizontalScrollBar(), &QScrollBar::valueChanged, ui->data_table->horizontalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->data_table->verticalScrollBar(), &QScrollBar::valueChanged, ui->incrementally->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(ui->incrementally->verticalScrollBar(), &QScrollBar::valueChanged, ui->data_table->verticalScrollBar(),
            &QScrollBar::setValue);

    connect(ui->data_table->horizontalHeader(), &QHeaderView::sectionResized, this,
            [this](int logicalIndex) { resizeTableCol(logicalIndex); });

    rightClickMenu = new QMenu(this);
    ui->data_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->data_table, &QTableView::customContextMenuRequested, this,
            &AnaliticFinance::customContextMenuRequested);
}

void AnaliticFinance::resizeColSum()
{
    for (int i = 0; i < mModel->columnCount(); i++)
    {
        //!!!!!!!!!!!!!!!!!!!!!!!! Обрабатывать только номера !!!!!!!!!!!!!!!!!!!!!
        if (mColumnNames[i].type == "number")
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

void AnaliticFinance::resizeTableCol(const int &ind)
{
    ui->column_sum->setColumnWidth(ind, ui->data_table->columnWidth(ind));
}

void AnaliticFinance::updateFilter()
{
    ui->column_sum->clear();
    QList<double> col_sum;
    QSqlQuery query;
    QString filter_text;
    QString sql = ui->filter_widget->filter();
    if (sql != "")
    {
        filter_text = " WHERE " + sql;
    }
    mModel->setFilter(sql);
    mIncModel->setQuery(Queries::remnants_incrementally.arg(filter_text));
    query.exec(Queries::sum_remnants_year + filter_text);
    while (query.next())
    {
        for (int i = 0; i < query.record().count(); i++)
        {
            col_sum.append(query.value(i).toDouble());
        }
    }
    for (int i = 0, j = 0; i < mModel->columnCount(); i++)
    {
        ui->column_sum->setColumnWidth(i, ui->data_table->columnWidth(i));  // сразу работаем с нижней строкой
        if (mColumnNames[i].type == "number")
        {
            ui->data_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            ui->column_sum->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
            int frac = static_cast<long long int>((col_sum[j] * 100)) % 100;
            QString num = QString::number(col_sum[j], 'f', (frac) ? 2 : 0);
            int shift = (col_sum[j] < 0) ? 1 : 0;
            for (int k = num.size() - ((frac) ? 6 : 3); k >= 1 + shift; k -= 3)
            {
                num.insert(k, ' ');
            }
            ui->column_sum->setItem(0, i, new QTableWidgetItem(num));
            j++;
        }
    }
    resizeColSum();
}

AnaliticFinance::~AnaliticFinance()
{
    delete ui;
}

void AnaliticFinance::customContextMenuRequested(const QPoint &pos)
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

void AnaliticFinance::on_action_add_triggered()
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

void AnaliticFinance::on_action_add_exp_part(const QString &exp_part)
{
    QString data = ExpensesPart::names.key(exp_part);
    if (ui->filter_widget->filter().isEmpty())
    {
        ui->filter_widget->clear();
    }
    ui->filter_widget->addRule(SqlExtendedFilterNode::WithoutInversion, SqlExtendedFilterNode::ActionEquals, data,
                               mModel->record().field("exp_part"));
}

void AnaliticFinance::add_exp_part_in_menu()
{
    QMenu *exp_part = rightClickMenu->addMenu("Выбор части расходов");
    QMap<QString, QString>::const_iterator i = ExpensesPart::names.constBegin();
    while (i != ExpensesPart::names.constEnd())
    {
        QAction *action = exp_part->addAction(i.value());
        connect(action, &QAction::triggered, this, [=]() { AnaliticFinance::on_action_add_exp_part(action->text()); });
        ++i;
    }
}
