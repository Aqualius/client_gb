#include "settings.h"

#include <LimeReport>
#include <QDate>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QUuid>

#include "sql_queries.h"
#include "ui_settings.h"

// QXLSX
#include "model_kkrb.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxchartsheet.h"
#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

Settings::Settings(QWidget *parent) : QWidget(parent), ui(new Ui::settings)
{
    ui->setupUi(this);

    setWindowTitle("Настройки");

    popUp = new PopUp(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    ui->year->setText(settings->value("settings/year").toString());

    // TODO: kgrbs (внимание КОСТЫЛЬ: ломается запись данных в ini файл, поэтому создается переменная kgrbs)
    QString kgrbs = settings->value("settings/kgrbs").toString();
    setDropDownTable(ui->cb_kgrbs, Queries::kkrb_kgrbs);
    ui->cb_kgrbs->setCurrentText(kgrbs);

    ui->path_update->setText(settings->value("pro_settings/path_update").toString());
}

Settings::~Settings()
{
    delete ui;
}

void Settings::setDropDownTable(QComboBox *rComboBox, const QString &rQuery)
{
    QSqlQuery query;
    query.prepare(rQuery);
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

    // установка кода по контенту внутри ячеек и рассчет для второго столбца
    // размера
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

void Settings::on_cb_kgrbs_currentIndexChanged(int index)
{
    ui->title_kgrbs->setText(ui->cb_kgrbs->model()->data(ui->cb_kgrbs->model()->index(index, 1)).toString());
    settings->setValue("settings/kgrbs", ui->cb_kgrbs->currentText());
    emit update_data();
}

void Settings::on_year_editingFinished()
{
    settings->setValue("settings/year", ui->year->text());
    emit update_data();
}

void Settings::on_browse_update_clicked()
{
    QString path = QFileDialog::getOpenFileName(0, "Открыть файл", QDir::currentPath(), "ZIP(*.zip)");
    if (path == "")
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Файл не был выбран!");
        popUp->show();
        return;
    }
    ui->path_update->setText(path);
    settings->setValue("pro_settings/path_update", path);
}

void Settings::on_pushButton_clicked()
{
    QString path =
        QFileDialog::getOpenFileName(0, "Открыть файл", "D:/Project/QT/client_gb/reports", "File Report(*.lrxml)");
    if (!path.isEmpty())
    {
        auto report = new LimeReport::ReportEngine(this);
        report->loadFromFile(path);
        report->designReport();
        popUp->setPopupType(Inforamation);
        popUp->setPopupText("Если отчеты уже есть в программе, то нужно пересобирать программу!");
        popUp->show();
    }
}

void Settings::on_pushButton_2_clicked()
{
    QSqlQuery query;
    QString status_uid;
    query.exec(Queries::select_status_plan_create);
    if (query.next())
    {
        status_uid = query.value("uid").toString();
    }

    QString path_to_save = QFileDialog::getOpenFileName(0, "Сохранить файл как", QDir::currentPath(), "EXCEL(*.xlsx)");
    if (path_to_save.isEmpty())
    {
        return;
    }
    Document xlsxR(path_to_save);

    if (xlsxR.load())
    {
        QSqlDatabase::database().transaction();
        // minus plan
        query.prepare(Queries::insert_plan_document);
        query.bindValue(":year_n", ui->year->text());
        query.bindValue(":kgrbs", ui->cb_kgrbs->currentText());
        query.bindValue(":doc_number", QVariant());
        query.bindValue(":doc_date", QVariant());
        query.bindValue(":status_uid", status_uid);
        query.bindValue(":basis_uid", QVariant());
        query.bindValue(":user_comment", "Перенос");
        query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));
        query.exec();
        int id = query.lastInsertId().toInt();

        if (!ui->krp_check->isChecked())
        {
            for (int row = 2;; row++)
            {
                if (xlsxR.cellAt(row, 1) == NULL)
                {
                    break;
                }
                QString old_ubp = xlsxR.cellAt(row, 1)->value().toString();
                QString new_ubp = xlsxR.cellAt(row, 2)->value().toString();
                QString kcs = xlsxR.cellAt(row, 3)->value().toString();

                // PLUS
                query.prepare(Queries::insert_plus_kcs);
                query.bindValue(":id", id);
                query.bindValue(":new_kubp", new_ubp);
                query.bindValue(":old_kubp", old_ubp);
                query.bindValue(":kcs", kcs);
                if (!query.exec())
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Не удалось добавить справку плюс в базу данных!");
                    popUp->show();
                    QSqlDatabase::database().rollback();
                    qCritical() << query.lastError();
                    return;
                }

                // MINUS
                query.prepare(Queries::insert_minus_kcs);
                query.bindValue(":id", id);
                query.bindValue(":new_kubp", new_ubp);
                query.bindValue(":old_kubp", old_ubp);
                query.bindValue(":kcs", kcs);
                if (!query.exec())
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Не удалось добавить справку минус в базу данных!");
                    popUp->show();
                    QSqlDatabase::database().rollback();
                    qCritical() << query.lastError();
                    return;
                }
            }
        }
        else
        {
            for (int row = 2;; row++)
            {
                if (xlsxR.cellAt(row, 1) == NULL)
                {
                    break;
                }
                QString old_ubp = xlsxR.cellAt(row, 1)->readValue().toString();
                QString new_ubp = xlsxR.cellAt(row, 2)->readValue().toString();
                QString krp = xlsxR.cellAt(row, 3)->readValue().toString();
                QString kcs = xlsxR.cellAt(row, 4)->readValue().toString();

                // PLUS
                query.prepare(Queries::insert_plus_krp_kcs);
                query.bindValue(":id", id);
                query.bindValue(":new_kubp", new_ubp);
                query.bindValue(":old_kubp", old_ubp);
                query.bindValue(":krp", krp);
                query.bindValue(":kcs", kcs);
                if (!query.exec())
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Не удалось добавить справку плюс в базу данных!");
                    popUp->show();
                    QSqlDatabase::database().rollback();
                    qCritical() << query.lastError();
                    return;
                }

                // MINUS
                query.prepare(Queries::insert_minus_krp_kcs);
                query.bindValue(":id", id);
                query.bindValue(":new_kubp", new_ubp);
                query.bindValue(":old_kubp", old_ubp);
                query.bindValue(":krp", krp);
                query.bindValue(":kcs", kcs);
                if (!query.exec())
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Не удалось добавить справку минус в базу данных!");
                    popUp->show();
                    QSqlDatabase::database().rollback();
                    qCritical() << query.lastError();
                    return;
                }
            }
        }
        QSqlDatabase::database().commit();
    }
    popUp->setPopupType(Access);
    popUp->setPopupText("Все ок. Открывайте редактор планов!");
    popUp->show();
}
