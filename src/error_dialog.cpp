#include "error_dialog.h"

#include <QDebug>
#include <QSqlQuery>

#include "delegate_expenses_part.h"
#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "sql_queries.h"
#include "table_view_utils.h"
#include "ui_error_dialog.h"

struct ColumnData
{
    QString name;
    QString type;
    int width;
};

ErrorDialog::ErrorDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);

    mPlanErrorModel = new QSqlQueryModel();
    ui->plan_table->setModel(mPlanErrorModel);
    mRemnErrorModel = new QSqlQueryModel();
    ui->remnants_table->setModel(mRemnErrorModel);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

bool ErrorDialog::check_plan(int id)
{
    QList<ColumnData> column_data = { { "Текст ошибки", "read_only", 200 }, { "Месяц", "read_only", 50 },
                                      { "КРП", "read_only", 39 },           { "КЦС", "read_only", 49 },
                                      { "КВР", "read_only", 39 },           { "КОСГУ", "read_only", 45 },
                                      { "КУБП", "read_only", 49 },          { "Часть расходов", "exp_part", 70 },
                                      { "Примечание", "read_only", 100 },   { "План", "number", 0 },
                                      { "Изменение", "number", 0 },         { "Разница", "number", 0 } };

    QSqlQuery query;
    query.prepare(Queries::check_plan);
    query.bindValue(":id", id);
    query.exec();
    mPlanErrorModel->setQuery(query);
    for (int i = 0; i < mPlanErrorModel->columnCount(); i++)
    {
        mPlanErrorModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        QString type = column_data[i].type;
        if (type == "read_only")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateReadOnly(this));
            if (column_data[i].name == "Примечание")
            {
                ui->plan_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            }
        }
        else if (type == "number")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
        }
        else if (type == "exp_part")
        {
            ui->plan_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
    }
    ui->plan_table->resizeColumnsToContents();

    column_data = { { "Текст ошибки", "read_only", 200 },
                    { "Месяц", "read_only", 50 },
                    { "КРП", "read_only", 39 },
                    { "КЦС", "read_only", 49 },
                    { "КВР", "read_only", 39 },
                    { "КОСГУ", "read_only", 45 },
                    { "КУБП", "read_only", 49 },
                    { "Часть расходов", "exp_part", 70 },
                    { "Примечание", "read_only", 100 },
                    { "План", "number", 0 },
                    { "Факт", "number", 0 },
                    { "Остаток", "number", 0 } };

    query.prepare(Queries::check_inc_plan);
    query.bindValue(":id", id);
    query.exec();
    mRemnErrorModel->setQuery(query);
    for (int i = 0; i < mRemnErrorModel->columnCount(); i++)
    {
        mRemnErrorModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        QString type = column_data[i].type;
        if (type == "read_only")
        {
            ui->remnants_table->setItemDelegateForColumn(i, new DelegateReadOnly(this));
            if (column_data[i].name == "Примечание")
            {
                ui->remnants_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            }
        }
        else if (type == "number")
        {
            ui->remnants_table->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
        }
        else if (type == "exp_part")
        {
            ui->remnants_table->setItemDelegateForColumn(i, new DelegateExpensesPart(this));
        }
    }
    ui->remnants_table->resizeColumnsToContents();

    if (mPlanErrorModel->rowCount() > 0 || mRemnErrorModel->rowCount() > 0)
    {
        TableViewUtils::applyStyle(ui->plan_table);
        TableViewUtils::applyStyle(ui->remnants_table);

        return false;
    }
    return true;
}
