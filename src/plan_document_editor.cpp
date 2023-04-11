#include "plan_document_editor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>

#include "sql_queries.h"
#include "ui_plan_document_editor.h"

PlanDocumentEditor::PlanDocumentEditor(QWidget *parent, int id)
    : QDialog(parent), mIdDoc(id), ui(new Ui::PlanDocumentEditor)
{
    ui->setupUi(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    mCurrentYear = settings->value("settings/year").toInt();
    mCurrentKgrbs = settings->value("settings/kgrbs").toString();

    if (mIdDoc == -1)
    {
        setWindowTitle("Добавление справки");
        ui->doc_date->setDate(QDate::currentDate());
        setDropDownTable(ui->basis_uid, Queries::select_all_bases);
    }
    else
    {
        setWindowTitle("Редактирование справки");
        QSqlQuery query;
        query.prepare(Queries::select_plan_document_with_id);
        query.bindValue(":id", mIdDoc);
        query.exec();
        while (query.next())
        {
            mCurrentDocNumber = query.value("doc_number").toString();
            ui->doc_date->setDate(query.value("doc_date").toDate());
            mCurrentUid = query.value("basis_uid").toString();
            ui->user_comment->setText(query.value("user_comment").toString());
        }
        setDropDownTable(ui->basis_uid, Queries::select_all_bases_edit);
        ui->basis_uid->setCurrentIndex(-1);
    }
}

PlanDocumentEditor::~PlanDocumentEditor()
{
    delete ui;
}

void PlanDocumentEditor::setDropDownTable(QComboBox *rComboBox, const QString &rQuery)
{
    rComboBox->setMaxVisibleItems(2000);
    QSqlQuery query;
    query.prepare(rQuery);
    query.bindValue(":year", mCurrentYear);
    query.bindValue(":kgrbs", mCurrentKgrbs);
    query.bindValue(":uid", mCurrentUid);

    query.exec();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(query);
    model->setHeaderData(0, Qt::Horizontal, "");
    model->setHeaderData(1, Qt::Horizontal, "uid");

    QTableView *table_view = new QTableView(this);
    rComboBox->setView(table_view);
    rComboBox->setModel(model);

    // скрываем колонку "uid"
    table_view->hideColumn(1);

    // скрыть номера строк
    table_view->verticalHeader()->hide();
    table_view->horizontalHeader()->hide();

    // установка кода по контенту внутри ячеек и рассчет для второго столбца размера
    table_view->setMinimumWidth(mWidthDropDown);
    table_view->setMinimumHeight(200);
    table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    // выбор всей строки, а не ячейки
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    rComboBox->setCurrentIndex(-1);
}

QString PlanDocumentEditor::docNumber() const
{
    return ui->doc_number->currentText();
}

QString PlanDocumentEditor::docDate() const
{
    return ui->doc_date->date().toString("yyyy-MM-dd");
}

QString PlanDocumentEditor::basisUid() const
{
    QModelIndex index = ui->basis_uid->model()->index(ui->basis_uid->currentIndex(), 3);
    return mCurrentUid;
}

QString PlanDocumentEditor::userComment() const
{
    return ui->user_comment->toPlainText();
}

void PlanDocumentEditor::accept()
{
    if (check_number())
    {
        int res = QMessageBox::question(this, "Вопрос", "Данный номер справки уже используется. Хотите продолжить?");
        if (res != QMessageBox::Yes)
        {
            return;
        }
    }
    QDialog::accept();
}

void PlanDocumentEditor::on_basis_uid_currentIndexChanged(int index)
{
    mCurrentUid = ui->basis_uid->model()->data(ui->basis_uid->model()->index(index, 1)).toString();

    // Добавление чисел соответствующего основания
    ui->doc_number->setMaxVisibleItems(2000);
    QSqlQuery query;
    query.prepare(Queries::select_all_bases_scope);
    query.bindValue(":year", mCurrentYear);
    query.bindValue(":kgrbs", mCurrentKgrbs);
    query.bindValue(":uid", mCurrentUid);

    query.exec();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(query);

    QTableView *table_view = new QTableView(this);
    table_view->horizontalHeader()->hide();
    table_view->verticalHeader()->hide();
    ui->doc_number->setView(table_view);
    ui->doc_number->setModel(model);

    table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    // выбор всей строки, а не ячейки
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->doc_number->setCurrentText(mCurrentDocNumber);
}

bool PlanDocumentEditor::check_number()
{
    QSqlQuery query;
    query.prepare(Queries::check_num_plan_doc);
    query.bindValue(":num", ui->doc_number->currentText());
    query.exec();
    if (query.next())
    {
        return true;
    }
    return false;
}

void PlanDocumentEditor::on_doc_number_currentIndexChanged(const QString &arg1)
{
    mCurrentDocNumber = arg1;
}
