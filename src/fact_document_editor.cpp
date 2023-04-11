#include "fact_document_editor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>

#include "sql_queries.h"
#include "ui_fact_document_editor.h"

FactDocumentEditor::FactDocumentEditor(QWidget *parent, int id)
    : QDialog(parent), mDocId(id), ui(new Ui::FactDocumentEditor)
{
    ui->setupUi(this);
    if (mDocId == -1)
    {
        setWindowTitle("Добавление заявки");
        ui->doc_date->setDate(QDate::currentDate());
    }
    else
    {
        setWindowTitle("Редактирование заявки");
        QSqlQuery query;
        query.prepare(Queries::select_fact_document_with_id);
        query.bindValue(":id", mDocId);
        query.exec();
        while (query.next())
        {
            ui->doc_number->setText(query.value("doc_number").toString());
            ui->doc_date->setDate(query.value("doc_date").toDate());
            ui->payment_type->setChecked(query.value("payment_type") == "REFUND");
            ui->user_comment->setText(query.value("user_comment").toString());
        }
    }
}

FactDocumentEditor::~FactDocumentEditor()
{
    delete ui;
}

QString FactDocumentEditor::docNumber()
{
    return ui->doc_number->text();
}

QString FactDocumentEditor::docDate()
{
    return ui->doc_date->date().toString("yyyy-MM-dd");
}

QString FactDocumentEditor::userComment()
{
    return ui->user_comment->toPlainText();
}

QString FactDocumentEditor::paymentType()
{
    return ui->payment_type->isChecked() ? "REFUND" : "FUND";
}

void FactDocumentEditor::accept()
{
    if (ui->doc_number->text() == "")
    {
        QMessageBox::warning(this, "Внимание", "Введите номер заявки!");
        return;
    }
    if (check_number())
    {
        int res = QMessageBox::question(this, "Вопрос", "Данный номер заявки уже используется. Хотите продолжить?");
        if (res != QMessageBox::Yes)
        {
            return;
        }
    }
    QDialog::accept();
}

bool FactDocumentEditor::check_number()
{
    QSqlQuery query;
    query.prepare(Queries::check_num_fact_doc);
    query.bindValue(":num", ui->doc_number->text());
    query.exec();
    if (query.next())
    {
        return true;
    }
    return false;
}
