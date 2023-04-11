#include "fact_document.h"

#include <LimeReport>
#include <QDebug>
#include <QDialog>
#include <QDirIterator>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include <QUuid>

#include "delegate_number_money.h"
#include "delegate_payment_type.h"
#include "delegate_readonly.h"
#include "delegate_status.h"
#include "editor_signature.h"
#include "fact_document_editor.h"
#include "sql_queries.h"
#include "ui_fact_document.h"
#include "xml_parser.h"

FactDocuments::FactDocuments(QWidget *parent) : QWidget(parent), ui(new Ui::FactDocument)
{
    ui->setupUi(this);

    popUp = new PopUp(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);

    mDocFactModel = new QSqlQueryModel(this);
    column_data = { { "id", "hidden", 0 },
                    { "Тип заявки", "payment_type", 100 },
                    { "Номер заявки", "read_only", 111 },
                    { "Дата заявки", "read_only", 98 },
                    { "Дата финанс", "read_only", 98 },
                    { "Комментарий", "read_only", 261 },
                    { "Статус документа", "status", 100 },
                    { "color", "hidden", 0 },
                    { "Сумма", "number", 100 } };

    update_data();

    ui->fact_documents->setModel(mDocFactModel);
    ui->fact_documents->setAlternatingRowColors(true);
    ui->filter_fact_doc->setColumnCount(mDocFactModel->columnCount());
    for (int i = 0; i < mDocFactModel->columnCount(); i++)
    {
        mDocFactModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        ui->fact_documents->setColumnWidth(i, column_data[i].width);
        ui->filter_fact_doc->setColumnWidth(i, column_data[i].width);
        QString type = column_data[i].type;
        if (type == "hidden")
        {
            ui->fact_documents->hideColumn(i);
            ui->filter_fact_doc->hideColumn(i);
        }
        else if (type == "payment_type")
        {
            ui->fact_documents->setItemDelegateForColumn(i, new DelegatePaymentType(this));
        }
        else if (type == "read_only")
        {
            ui->fact_documents->setItemDelegateForColumn(i, new DelegateReadOnly(this));
        }
        else if (type == "status")
        {
            ui->fact_documents->setItemDelegateForColumn(i, new DelegateStatus(this));
        }
        else if (type == "number")
        {
            ui->fact_documents->setItemDelegateForColumn(i, new DelegateNumberMoney(this));
        }
    }
    ui->fact_documents->resizeColumnsToContents();
    resizeFilter();

    connect(ui->fact_documents->horizontalScrollBar(), &QScrollBar::valueChanged,
            ui->filter_fact_doc->horizontalScrollBar(), &QScrollBar::setValue);
    connect(ui->filter_fact_doc->horizontalScrollBar(), &QScrollBar::valueChanged,
            ui->fact_documents->horizontalScrollBar(), &QScrollBar::setValue);

    connect(ui->fact_documents->horizontalHeader(), &QHeaderView::sectionResized, this, &FactDocuments::resizeFilter);

    ui->fact_documents->setFocus();

    ////////// ГОРЯЧИЕ КЛАВИШИ /////////
    QShortcut *shortcut;
    // open
    shortcut = new QShortcut(Qt::Key_Return, ui->fact_documents);
    shortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(shortcut, &QShortcut::activated, this, &FactDocuments::on_button_open_clicked);

    shortcut = new QShortcut(Qt::Key_Enter, ui->fact_documents);
    shortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(shortcut, &QShortcut::activated, this, &FactDocuments::on_button_open_clicked);
}

FactDocuments::~FactDocuments()
{
    delete ui;
}

void FactDocuments::resizeFilter()
{
    for (int i = 1; i < mDocFactModel->columnCount(); i++)
    {
        ui->filter_fact_doc->setColumnWidth(i, ui->fact_documents->columnWidth(i));
    }
}

void FactDocuments::setFilter()
{
    QString filter = "";
    for (int col = 0; col < mDocFactModel->columnCount(); col++)
    {
        if (ui->filter_fact_doc->item(0, col))
        {
            QString text = ui->filter_fact_doc->item(0, col)->text().replace("'", "''");
            if (column_data[col].type == "payment_type")
            {
                filter += " AND payment_type = ";
                if (QString("Финансирование").contains(text))
                {
                    filter += "'FUND'";
                }
                else if (QString("Возврат").contains(text))
                {
                    filter += "'REFUND'";
                }
                else
                {
                    filter += "''";
                }
            }
            else if (text != "")
            {
                QString tmp = mDocFactModel->record().fieldName(col);
                if (mDocFactModel->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString() == "Статус")
                {
                    tmp = "s." + tmp;
                }
                else if (mDocFactModel->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString() == "Сумма")
                {
                    // TODO: Не обрабатывается фильтр для суммы
                    continue;
                }
                filter += QString(" AND %1 LIKE '%%2%'").arg(tmp, text);
            }
        }
    }
    QSqlQuery query;
    query.prepare(Queries::select_fact_document.arg(filter));
    query.bindValue(":kgrbs", mCurrentKgrbs);
    query.bindValue(":year_n", mCurrentYear);
    query.exec();
    mDocFactModel->setQuery(query);
}

void FactDocuments::update_data()
{
    mCurrentYear = settings->value("settings/year").toInt();
    mCurrentKgrbs = settings->value("settings/kgrbs").toString();
    setFilter();
    ui->fact_documents->resizeColumnsToContents();
    resizeFilter();
}

void FactDocuments::on_filter_fact_doc_cellChanged(int row, int column)
{
    setFilter();
}

void FactDocuments::on_button_open_clicked()
{
    if (ui->fact_documents->selectionModel()->selectedRows().size() != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну заявку из списка!");
        popUp->show();
        return;
    }
    const QAbstractItemModel *model = ui->fact_documents->currentIndex().model();
    const int row = ui->fact_documents->currentIndex().row();
    emit factDocumentActivated(mDocFactModel->data(model->index(row, 0)).toInt(),
                               mDocFactModel->data(model->index(row, 6)).toString() != "Черновик");
}

void FactDocuments::on_fact_documents_doubleClicked(const QModelIndex &index)
{
    QTimer::singleShot(0, this, &FactDocuments::on_button_open_clicked);
}

void FactDocuments::on_button_add_clicked()
{
    FactDocumentEditor create_doc(this);
    if (create_doc.exec() == QDialog::Accepted)
    {
        // Get "Create status"
        QString status_uid = "";
        QSqlQuery query;
        query.exec(Queries::select_status_fact_create);
        if (query.next())
        {
            status_uid = query.value("uid").toString();
        }
        query.prepare(Queries::insert_fact_document);
        query.bindValue(":year_n", mCurrentYear);
        query.bindValue(":kgrbs", mCurrentKgrbs);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate().isEmpty() ? QVariant() : create_doc.docDate());
        query.bindValue(":status_uid", status_uid);
        query.bindValue(":payment_type", create_doc.paymentType());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());
        query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось добавить заявку в базу данных.");
            popUp->show();
            qCritical() << query.lastError();
            return;
        }
        setFilter();
    }
}

void FactDocuments::on_button_edit_clicked()
{
    if (ui->fact_documents->selectionModel()->selectedRows().size() != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну заявку из списка!");
        popUp->show();
        return;
    }
    const QAbstractItemModel *model = ui->fact_documents->currentIndex().model();
    const int row = ui->fact_documents->currentIndex().row();
    if (mDocFactModel->data(model->index(row, 6)).toString() != "Черновик")
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Вы не можете изменять уже проведенные заявки!");
        popUp->show();
        return;
    }
    int id = mDocFactModel->data(mDocFactModel->index(ui->fact_documents->currentIndex().row(), 0)).toInt();
    FactDocumentEditor create_doc(this, id);
    if (create_doc.exec() == QDialog::Accepted)
    {
        QSqlQuery query;
        query.prepare(Queries::update_fact_document);
        query.bindValue(":id", id);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());
        query.bindValue(":payment_type", create_doc.paymentType());

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Ошибка при обновлении заявки в базе данных.");
            popUp->show();
            qCritical() << query.lastError();
            return;
        }
        setFilter();
    }
}

void FactDocuments::on_button_del_clicked()
{
    if (ui->fact_documents->selectionModel()->selectedRows().count() <= 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна заявка не выбрана!");
        popUp->show();
        return;
    }
    const QList<QModelIndex> selected_rows = ui->fact_documents->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected_rows)
    {
        if (mDocFactModel->data(mDocFactModel->index(index.row(), 6)).toString() != "Черновик")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Вы можете удалять только черновые заявки!");
            popUp->show();
            return;
        }
    }
    int res = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите удалить все выделенные строки?");
    if (res == QMessageBox::Yes)
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.transaction())
        {
            bool check_error = false;
            QSqlQuery query;
            query.prepare(Queries::delete_in_fact_document);
            for (const QModelIndex &index : selected_rows)
            {
                query.bindValue(":id", mDocFactModel->data(mDocFactModel->index(index.row(), 0)));
                if (!query.exec())
                {
                    qCritical() << query.lastError();
                    check_error = true;
                    break;
                }
            }

            if (!db.commit() || check_error)
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось удалить заявки!");
                popUp->show();
                qCritical() << "Failed to commit";
                db.rollback();
                return;
            }
        }
        else
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось начать удаление заявок!");
            popUp->show();
            qCritical() << "Failed to start transaction mode";
            return;
        }
        setFilter();
        popUp->setPopupType(Access);
        popUp->setPopupText("Заявки успешно удалены!");
        popUp->show();
    }
}

void FactDocuments::on_button_exp_clicked()
{
    int selectCount = ui->fact_documents->selectionModel()->selectedRows().count();
    QString path_to_save;
    if (selectCount == 1)
    {
        int ind_row = ui->fact_documents->selectionModel()->selectedRows().at(0).row();
        int id = mDocFactModel->data(mDocFactModel->index(ind_row, 0)).toInt();
        QString number_doc = mDocFactModel->data(mDocFactModel->index(ind_row, 2)).toString();
        path_to_save = QFileDialog::getSaveFileName(
            0, "Сохранить файл как",
            QDir::currentPath() + QString("\\ex_fact_doc_%2_%1.xml").arg(number_doc, mCurrentKgrbs), "XML(*.xml)");
        if (path_to_save == "")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Файл не был выбран!");
            popUp->show();
            return;
        }
        XmlService parse(path_to_save);
        if (!parse.fact_export(id))
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Не все файлы были сохранены!");
            popUp->show();
            return;
        }
        popUp->setPopupType(Access);
        popUp->setPopupText("Файл сохранен по пути\n" + path_to_save);
        popUp->show();
    }
    else if (selectCount > 1)
    {
        path_to_save = QFileDialog::getExistingDirectory(0, ("Выбор папки"), QDir::currentPath());
        if (path_to_save == "")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Папка не была выбрана!");
            popUp->show();
            return;
        }
        XmlService parse(path_to_save);
        bool success = true;
        const auto selected_rows = ui->fact_documents->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_rows)
        {
            int id = mDocFactModel->data(mDocFactModel->index(index.row(), 0)).toInt();
            if (!parse.fact_export(id))
            {
                success = false;
            }
        }
        if (!success)
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Не все файлы были сохранены!");
            popUp->show();
            return;
        }
        popUp->setPopupType(Access);
        popUp->setPopupText("Файлы сохранены в папку\n" + path_to_save);
        popUp->show();
    }
    else
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна заявка не выбрана!");
        popUp->show();
    }
}

void FactDocuments::on_button_imp_clicked()
{
    QStringList path_imp = QFileDialog::getOpenFileNames(0, "Открыть файл(ы)", QDir::currentPath(), "XML(*.xml)");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (path_imp.empty())
    {
        QApplication::restoreOverrideCursor();
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни один файл не был выбран!");
        popUp->show();
        return;
    }
    XmlService parse("");

    QString path_error;
    for (const QString &path : qAsConst(path_imp))
    {
        if (!parse.fact_import(path))
        {
            path_error += QString("\n%1 - %2").arg(path.split('/').last(), parse.lastError());
        }
    }
    setFilter();
    if (path_error != "")
    {
        QApplication::restoreOverrideCursor();
        popUp->setPopupType(Warning);
        popUp->setPopupText("Не все заявки удалось добавить. Список заявок, которые не удалось затянуть:" + path_error);
        popUp->show();
        qWarning() << "Не все заявки удалось добавить. Список заявок, которые не удалось затянуть:" + path_error;
        return;
    }
    QApplication::restoreOverrideCursor();
    popUp->setPopupType(Access);
    popUp->setPopupText("Импорт успешно выполнен!");
    popUp->show();
}

void FactDocuments::on_button_dub_clicked()
{
    int selectCount = ui->fact_documents->selectionModel()->selectedRows().count();
    if (selectCount == 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна заявка не выбрана!");
        popUp->show();
        return;
    }
    int res = QMessageBox::question(
        this, "Подтверждение", "Вы хотите дублировать заявки? (ВНИМАНИЕ: дублироваться будут все выделенные заявки)");
    if (res == QMessageBox::Yes)
    {
        QSqlDatabase sdb = QSqlDatabase::database();
        sdb.transaction();
        const auto selected_rows = ui->fact_documents->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_rows)
        {
            int old_id = mDocFactModel->data(mDocFactModel->index(index.row(), 0)).toInt();
            QSqlQuery query;
            query.prepare(Queries::duplicate_fact_doc);
            query.bindValue(":id", old_id);
            query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось дублировать заявки!");
                popUp->show();
                qCritical() << query.lastError();
                sdb.rollback();
                return;
            }
            int id = query.lastInsertId().toInt();
            query.prepare(Queries::duplicate_fact_data);
            query.bindValue(":id", old_id);
            query.bindValue(":document_id", id);
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось дублировать заявки!");
                popUp->show();
                qCritical() << query.lastError();
                sdb.rollback();
                return;
            }
        }

        if (!sdb.commit())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось дублировать заявки!");
            popUp->show();
            qCritical() << sdb.lastError();
            sdb.rollback();
        }
        setFilter();
        QString end_message;
        if (selectCount / 10 % 10 == 1 || selectCount % 10 > 4 || selectCount % 10 == 0)
        {
            end_message = "заявок";
        }
        else if (selectCount % 10 == 1)
        {
            end_message = "заявка";
        }
        else
        {
            end_message = "заявки";
        }
        popUp->setPopupType(Access);
        popUp->setPopupText(QString("Удалось дублировать %1 %2!").arg(selectCount).arg(end_message));
        popUp->show();
    }
}

void FactDocuments::on_button_rep_clicked()
{
    int selectCount = ui->fact_documents->selectionModel()->selectedRows().count();
    if (selectCount != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну заявку!");
        popUp->show();
        return;
    }
    EditorSignature sign_edit(ReportType::FACT, this);
    if (sign_edit.exec() == QDialog::Rejected)
    {
        return;
    }
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    auto report = new LimeReport::ReportEngine(this);
    report->dataManager()->setReportVariable(
        "id_doc", mDocFactModel->data(mDocFactModel->index(ui->fact_documents->currentIndex().row(), 0)).toInt());
    report->dataManager()->setReportVariable("signatory_1", settings->value("report_fact/fio_1").toString());
    report->dataManager()->setReportVariable("signatory_2", settings->value("report_fact/fio_2").toString());
    report->dataManager()->setReportVariable("signatory_3", settings->value("report_fact/fio_3").toString());
    report->dataManager()->setReportVariable("signatory_post_1", settings->value("report_fact/post_1").toString());
    report->dataManager()->setReportVariable("signatory_post_2", settings->value("report_fact/post_2").toString());
    report->dataManager()->setReportVariable("signatory_post_3", settings->value("report_fact/post_3").toString());
    report->dataManager()->setReportVariable("signatory_tel_1", settings->value("report_fact/tel_1").toString());
    report->dataManager()->setReportVariable("signatory_tel_2", settings->value("report_fact/tel_2").toString());
    report->dataManager()->setReportVariable("signatory_tel_3", settings->value("report_fact/tel_3").toString());
    report->loadFromFile(":/reports/zayavka_on_fact.lrxml");

    QApplication::restoreOverrideCursor();
    report->previewReport();
    QWidget::setEnabled(true);
}

void FactDocuments::on_button_rep_alter_clicked()
{
    int selectCount = ui->fact_documents->selectionModel()->selectedRows().count();
    if (selectCount != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну заявку!");
        popUp->show();
        return;
    }
    EditorSignature sign_edit(ReportType::FACT, this);
    if (sign_edit.exec() == QDialog::Rejected)
    {
        return;
    }
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    auto report = new LimeReport::ReportEngine(this);
    report->dataManager()->setReportVariable(
        "id_doc", mDocFactModel->data(mDocFactModel->index(ui->fact_documents->currentIndex().row(), 0)).toInt());
    report->dataManager()->setReportVariable("signatory_1", settings->value("report_fact/fio_1").toString());
    report->dataManager()->setReportVariable("signatory_2", settings->value("report_fact/fio_2").toString());
    report->dataManager()->setReportVariable("signatory_3", settings->value("report_fact/fio_3").toString());
    report->dataManager()->setReportVariable("signatory_post_1", settings->value("report_fact/post_1").toString());
    report->dataManager()->setReportVariable("signatory_post_2", settings->value("report_fact/post_2").toString());
    report->dataManager()->setReportVariable("signatory_post_3", settings->value("report_fact/post_3").toString());
    report->dataManager()->setReportVariable("signatory_tel_1", settings->value("report_fact/tel_1").toString());
    report->dataManager()->setReportVariable("signatory_tel_2", settings->value("report_fact/tel_2").toString());
    report->dataManager()->setReportVariable("signatory_tel_3", settings->value("report_fact/tel_3").toString());
    report->loadFromFile(":/reports/zayavka_on_fact_alter.lrxml");

    QApplication::restoreOverrideCursor();
    report->previewReport();
    QWidget::setEnabled(true);
}

void FactDocuments::on_button_send_clicked()
{
    int selectCount = ui->fact_documents->selectionModel()->selectedRows().count();
    QString path_to_save = QDir::currentPath() + "/send";
    QDir dir(path_to_save);
    if (!dir.mkdir(path_to_save))
    {
        QDirIterator it(path_to_save, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QFile::remove(it.next());
        }
    }
    if (selectCount <= 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна заявка не выбрана!");
        popUp->show();
        return;
    }
    XmlService xml_service(path_to_save);
    bool success = true;
    const auto selected_rows = ui->fact_documents->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected_rows)
    {
        int id = mDocFactModel->data(mDocFactModel->index(index.row(), 0)).toInt();
        if (!xml_service.fact_export(id))
        {
            success = false;
        }
        if (mDocFactModel->data(mDocFactModel->index(index.row(), 6)).toString() != "Черновик")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Вы можете отправлять только черновые заявки!");
            popUp->show();
            return;
        }
    }
    if (!success)
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Не удалось сформировать файлы для отправки. Детали:\n" + xml_service.lastError());
        popUp->show();
        dir.rmpath(path_to_save);
        return;
    }
    QString files_name;
    dir.rmpath(path_to_save);
    QDirIterator it(path_to_save, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        files_name += it.next() + ";";
    }

    QSettings settings("settings.ini", QSettings::IniFormat);
    QString program_path = "./gbupd/GBUpdater.exe";
    QStringList arguments;
    arguments << "-year" << settings.value("settings/year").toString() << "-sendfiles" << files_name
              << "-sendfiles-recp"
              << "99";
    QProcess *process = new QProcess(this);
    process->setWorkingDirectory("./gbupd");
    process->start(program_path, arguments);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit)
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Процесс отправки завершен некорректно. Детали:\n" + process->errorString());
                    popUp->show();
                    return;
                }
                else if (exitCode != 0)
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Процесс отправки завершен некорректно. Детали:\n" + process->errorString());
                    popUp->show();
                    return;
                }
                const QStringList f = files_name.split(';');
                for (const QString &file : f)
                {
                    if (!file.isEmpty())
                    {
                        QFile::remove(file);
                    }
                }
                dir.rmpath(path_to_save);
                emit FactDocuments::updateDB();
            });
}

void FactDocuments::on_button_merge_clicked()
{
    if (ui->fact_documents->selectionModel()->selectedRows().count() <= 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите несколько справок!");
        popUp->show();
        return;
    }
    FactDocumentEditor create_doc(this);
    if (create_doc.exec() == QDialog::Accepted)
    {
        // Get "Create status"
        QString status_uid = "";
        QSqlQuery query;
        query.exec(Queries::select_status_fact_create);
        if (query.next())
        {
            status_uid = query.value("uid").toString();
        }
        QSqlDatabase::database().transaction();
        query.prepare(Queries::insert_fact_document);
        query.bindValue(":year_n", mCurrentYear);
        query.bindValue(":kgrbs", mCurrentKgrbs);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate().isEmpty() ? QVariant() : create_doc.docDate());
        query.bindValue(":status_uid", status_uid);
        query.bindValue(":payment_type", create_doc.paymentType());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());
        query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось добавить заявку в базу данных!");
            popUp->show();
            QSqlDatabase::database().rollback();
            qCritical() << query.lastError();
            return;
        }

        int new_id = query.lastInsertId().toInt();
        const auto selected_rows = ui->fact_documents->selectionModel()->selectedRows();
        query.prepare(Queries::insert_merge_in_exp_fact);
        query.bindValue(":new_id", new_id);
        for (const QModelIndex &index : selected_rows)
        {
            int old_id = mDocFactModel->data(mDocFactModel->index(index.row(), 0)).toInt();
            query.bindValue(":old_id", old_id);
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось добавить данные в заявку!");
                popUp->show();
                QSqlDatabase::database().rollback();
                qCritical() << query.lastError();
                return;
            }
        }

        QSqlDatabase::database().commit();
        setFilter();
    }
}
