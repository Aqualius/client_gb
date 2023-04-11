#include "plan_document.h"

#include <LimeReport>
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QMdiArea>
#include <QMessageBox>
#include <QPair>
#include <QProcess>
#include <QScrollBar>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include <QUuid>

#include "delegate_number_money.h"
#include "delegate_readonly.h"
#include "delegate_status.h"
#include "editor_signature.h"
#include "json_service.h"
#include "plan_document_editor.h"
#include "sql_queries.h"
#include "ui_plan_document.h"
#include "xml_parser.h"

PlanDocuments::PlanDocuments(QWidget *parent) : QWidget(parent), ui(new Ui::PlanDocument)
{
    ui->setupUi(this);

    popUp = new PopUp(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);

    mDocPlanModel = new QSqlQueryModel(this);
    column_data = { { "id", "hidden", 0 },
                    { "Номер", "read_only", 111 },
                    { "Дата", "read_only", 98 },
                    { "Тип основания", "read_only", 221 },
                    { "Номер основания", "read_only", 156 },
                    { "Дата основания", "read_only", 200 },
                    { "Комментарий", "read_only", 261 },
                    { "Статус документа", "status", 100 },
                    { "color", "hidden", 0 } };

    update_data();

    ui->plan_documents->setModel(mDocPlanModel);
    ui->plan_documents->setAlternatingRowColors(true);

    ui->filter_plan_doc->setColumnCount(mDocPlanModel->columnCount());
    for (int i = 0; i < mDocPlanModel->columnCount(); i++)
    {
        mDocPlanModel->setHeaderData(i, Qt::Horizontal, column_data[i].name);
        ui->plan_documents->setColumnWidth(i, column_data[i].width);
        ui->filter_plan_doc->setColumnWidth(i, column_data[i].width);
        QString type = column_data[i].type;
        if (type == "hidden")
        {
            ui->plan_documents->hideColumn(i);
            ui->filter_plan_doc->hideColumn(i);
        }
        else if (type == "read_only")
        {
            ui->plan_documents->setItemDelegateForColumn(i, new DelegateReadOnly(this));
        }
        else if (type == "status")
        {
            ui->plan_documents->setItemDelegateForColumn(i, new DelegateStatus(this));
        }
    }

    ui->plan_documents->resizeColumnsToContents();
    resizeFilter();
    setFilter();
    ui->plan_documents->setFocus();

    connect(ui->plan_documents->horizontalScrollBar(), &QScrollBar::valueChanged,
            ui->filter_plan_doc->horizontalScrollBar(), &QScrollBar::setValue);
    connect(ui->filter_plan_doc->horizontalScrollBar(), &QScrollBar::valueChanged,
            ui->plan_documents->horizontalScrollBar(), &QScrollBar::setValue);

    connect(ui->plan_documents->horizontalHeader(), &QHeaderView::sectionResized, this, &PlanDocuments::resizeFilter);

    ////////// ГОРЯЧИЕ КЛАВИШИ /////////
    QShortcut *shortcut;
    // open
    shortcut = new QShortcut(Qt::Key_Return, ui->plan_documents);
    shortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(shortcut, &QShortcut::activated, this, &PlanDocuments::on_button_open_clicked);

    shortcut = new QShortcut(Qt::Key_Enter, ui->plan_documents);
    shortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(shortcut, &QShortcut::activated, this, &PlanDocuments::on_button_open_clicked);
}

PlanDocuments::~PlanDocuments()
{
    delete ui;
}

void PlanDocuments::resizeFilter()
{
    for (int i = 1; i < mDocPlanModel->columnCount(); i++)
    {
        ui->filter_plan_doc->setColumnWidth(i, ui->plan_documents->columnWidth(i));
    }
}

void PlanDocuments::setFilter()
{
    QString filter = "";
    for (int col = 0; col < mDocPlanModel->columnCount(); col++)
    {
        if (ui->filter_plan_doc->item(0, col))
        {
            QString text = ui->filter_plan_doc->item(0, col)->text().replace("'", "''");
            if (text != "")
            {
                QString tmp = mDocPlanModel->record().fieldName(col);
                if (mDocPlanModel->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString() == "Статус")
                {
                    tmp = "s." + tmp;
                }
                filter += QString(" AND %1 LIKE '%%2%'").arg(tmp, text);
            }
        }
    }
    QSqlQuery query;
    query.prepare(Queries::select_plan_document.arg(filter));
    query.bindValue(":kgrbs", mCurrentKgrbs);
    query.bindValue(":year_n", mCurrentYear);
    query.exec();
    mDocPlanModel->setQuery(query);
}

void PlanDocuments::update_data()
{
    mCurrentYear = settings->value("settings/year").toInt();
    mCurrentKgrbs = settings->value("settings/kgrbs").toString();
    setFilter();
    ui->plan_documents->resizeColumnsToContents();
    resizeFilter();
}

void PlanDocuments::on_filter_plan_doc_cellChanged(int row, int column)
{
    setFilter();
}

void PlanDocuments::on_button_open_clicked()
{
    if (ui->plan_documents->selectionModel()->selectedRows().size() != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну справку из списка!");
        popUp->show();
        return;
    }
    const QAbstractItemModel *model = ui->plan_documents->currentIndex().model();
    const int row = ui->plan_documents->currentIndex().row();

    emit planDocumentActivated(mDocPlanModel->data(model->index(row, 0)).toInt(),
                               mDocPlanModel->data(model->index(row, 7)).toString() != "Черновик");
}

void PlanDocuments::on_plan_documents_doubleClicked(const QModelIndex &index)
{
    QTimer::singleShot(0, this, &PlanDocuments::on_button_open_clicked);
}

void PlanDocuments::on_button_add_clicked()
{
    // Get "Create status"
    QString status_uid;
    QSqlQuery query;
    query.exec(Queries::select_status_plan_create);
    if (query.next())
    {
        status_uid = query.value("uid").toString();
    }
    PlanDocumentEditor create_doc(this);
    if (create_doc.exec() == QDialog::Accepted)
    {
        query.prepare(Queries::insert_plan_document);
        query.bindValue(":year_n", mCurrentYear);
        query.bindValue(":kgrbs", mCurrentKgrbs);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate());
        query.bindValue(":status_uid", status_uid);
        query.bindValue(":basis_uid", create_doc.basisUid().isEmpty() ? QVariant() : create_doc.basisUid());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());
        query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось добавить справку в базу данных!");
            popUp->show();
            qCritical() << query.lastError();
            return;
        }
        setFilter();
    }
}

void PlanDocuments::on_button_edit_clicked()
{
    if (ui->plan_documents->selectionModel()->selectedRows().size() != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну справку из списка!");
        popUp->show();
        return;
    }
    const QAbstractItemModel *model = ui->plan_documents->currentIndex().model();
    const int row = ui->plan_documents->currentIndex().row();
    if (mDocPlanModel->data(model->index(row, 7)).toString() != "Черновик")
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Вы можете изменять только черновые справки!");
        popUp->show();
        return;
    }
    int id = mDocPlanModel->data(mDocPlanModel->index(ui->plan_documents->currentIndex().row(), 0)).toInt();
    PlanDocumentEditor create_doc(this, id);
    if (create_doc.exec() == QDialog::Accepted)
    {
        QSqlQuery query;
        query.prepare(Queries::update_plan_document);
        query.bindValue(":id", id);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate());
        query.bindValue(":basis_uid", create_doc.basisUid().isEmpty() ? QVariant() : create_doc.basisUid());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Ошибка при обновлении справки.");
            popUp->show();
            qCritical() << query.lastError();
            return;
        }
    }
    setFilter();
}

void PlanDocuments::on_button_del_clicked()
{
    if (ui->plan_documents->selectionModel()->selectedRows().count() <= 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна справка не выбрана!");
        popUp->show();
        return;
    }
    const QList<QModelIndex> selected_rows = ui->plan_documents->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected_rows)
    {
        if (mDocPlanModel->data(mDocPlanModel->index(index.row(), 7)).toString() != "Черновик")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Вы не можете удалять уже проведенные справки!");
            popUp->show();
            return;
        }
    }
    int res = QMessageBox::question(this, "Подтверждение",
                                    "Вы хотите удалить справки? (ВНИМАНИЕ: удаляются все выделенные справки)");
    if (res == QMessageBox::Yes)
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.transaction())
        {
            bool check_error = false;
            QSqlQuery query;
            for (const QModelIndex &index : selected_rows)
            {
                int id = mDocPlanModel->data(mDocPlanModel->index(index.row(), 0)).toInt();
                query.prepare(Queries::delete_in_plan_document);
                query.bindValue(":id", id);
                if (!query.exec())
                {
                    check_error = true;
                    break;
                }
            }

            if (!db.commit() || check_error)
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось удалить справки!");
                popUp->show();
                qCritical() << "Failed to commit";
                db.rollback();
                return;
            }
        }
        else
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось начать удаление справок!");
            popUp->show();
            qCritical() << "Failed to start transaction mode";
            return;
        }
        setFilter();
        popUp->setPopupType(Access);
        popUp->setPopupText("Справки успешно удалены!");
        popUp->show();
    }
}

void PlanDocuments::on_button_exp_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
    QString path_to_save;
    if (selectCount == 1)
    {
        int ind_row = ui->plan_documents->selectionModel()->selectedRows().at(0).row();
        int id = mDocPlanModel->data(mDocPlanModel->index(ind_row, 0)).toInt();
        QString number_doc = mDocPlanModel->data(mDocPlanModel->index(ind_row, 1)).toString();
        path_to_save = QFileDialog::getSaveFileName(
            0, "Сохранить файл как",
            QDir::currentPath() + QString("\\ex_plan_doc_%2_%1.xml").arg(number_doc, mCurrentKgrbs), "XML(*.xml)");
        if (path_to_save == "")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Файл не был выбран!");
            popUp->show();
            return;
        }
        XmlService xml_service(path_to_save);
        if (xml_service.plan_export(id))
        {
            popUp->setPopupType(Access);
            popUp->setPopupText("Файл сохранен по пути\n" + path_to_save);
            popUp->show();
        }
        else
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Ошибка при экспорте. Детали:\n" + xml_service.lastError());
            popUp->show();
            return;
        }
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
        const auto selected_rows = ui->plan_documents->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_rows)
        {
            int id = mDocPlanModel->data(mDocPlanModel->index(index.row(), 0)).toInt();
            if (!parse.plan_export(id))
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
        popUp->setPopupText("Ни одна справка не выбрана!");
        popUp->show();
        return;
    }
}

void PlanDocuments::on_button_imp_clicked()
{
    QStringList path_imp = QFileDialog::getOpenFileNames(0, "Открыть файл(ы)", QDir::currentPath(), "XML(*.xml)");
    if (path_imp.empty())
    {
        QApplication::restoreOverrideCursor();
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни один файл не выбран!");
        popUp->show();
        return;
    }
    XmlService parse("");

    QString path_error;
    for (const QString &path : qAsConst(path_imp))
    {
        if (!parse.plan_import(path))
        {
            path_error += QString("\n%1 - %2").arg(path.split('/').last(), parse.lastError());
        }
    }

    if (path_error != "")
    {
        QApplication::restoreOverrideCursor();
        popUp->setPopupType(Warning);
        popUp->setPopupText("Не все справки удалось добавить. Список справок, которые не удалось затянуть:" +
                            path_error);
        popUp->show();
        qWarning() << "Не все справки удалось добавить. Список справок, которые не удалось затянуть:" + path_error;
        setFilter();
        return;
    }

    QApplication::restoreOverrideCursor();
    popUp->setPopupType(Access);
    popUp->setPopupText("Импорт успешно выполнен!");
    popUp->show();
    setFilter();
}

void PlanDocuments::on_button_dub_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
    if (selectCount == 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Ни одна справка не выбрана!");
        popUp->show();
        return;
    }
    int res = QMessageBox::question(
        this, "Подтверждение", "Вы хотите дублировать справки? (ВНИМАНИЕ: дублироваться будут все выделенные справки)");
    if (res == QMessageBox::Yes)
    {
        QSqlDatabase sdb = QSqlDatabase::database();
        sdb.transaction();
        const auto selected_rows = ui->plan_documents->selectionModel()->selectedRows();
        for (const QModelIndex &index : selected_rows)
        {
            int old_id = mDocPlanModel->data(mDocPlanModel->index(index.row(), 0)).toInt();
            QSqlQuery query;
            query.prepare(Queries::duplicate_plan_doc);
            query.bindValue(":id", old_id);
            query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось начать дублирование справок!");
                popUp->show();
                qCritical() << query.lastError();
                sdb.rollback();
                return;
            }
            int id = query.lastInsertId().toInt();
            query.prepare(Queries::duplicate_plan_data);
            query.bindValue(":id", old_id);
            query.bindValue(":document_id", id);
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось начать дублирование справок!");
                popUp->show();
                qCritical() << query.lastError();
                sdb.rollback();
                return;
            }
        }

        if (!sdb.commit())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось начать дублирование справок!");
            popUp->show();
            qCritical() << sdb.lastError();
            sdb.rollback();
        }
        setFilter();
        popUp->setPopupType(Access);
        popUp->setPopupText(QString("Удалось дублировать %1 справки!").arg(selectCount));
        popUp->show();
    }
}

void PlanDocuments::on_button_rep_with_kkrb_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
    if (selectCount != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну справку!");
        popUp->show();
        return;
    }
    EditorSignature sign_edit(ReportType::PLAN, this);
    if (sign_edit.exec() == QDialog::Rejected)
    {
        return;
    }
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    auto report = new LimeReport::ReportEngine(this);
    report->dataManager()->setReportVariable(
        "id_doc", mDocPlanModel->data(mDocPlanModel->index(ui->plan_documents->currentIndex().row(), 0)).toInt());
    report->dataManager()->setReportVariable("signatory_1", settings->value("report_plan/fio_1").toString());
    report->dataManager()->setReportVariable("signatory_2", settings->value("report_plan/fio_2").toString());
    report->dataManager()->setReportVariable("signatory_3", settings->value("report_plan/fio_3").toString());
    report->dataManager()->setReportVariable("signatory_post_1", settings->value("report_plan/post_1").toString());
    report->dataManager()->setReportVariable("signatory_post_2", settings->value("report_plan/post_2").toString());
    report->dataManager()->setReportVariable("signatory_post_3", settings->value("report_plan/post_3").toString());
    report->loadFromFile(":/reports/spravka_on_plan_kkrb_svod.lrxml");

    QApplication::restoreOverrideCursor();
    report->previewReport();
    QWidget::setEnabled(true);
}

void PlanDocuments::on_button_rep_with_kosgu_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
    if (selectCount != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну справку!");
        popUp->show();
        return;
    }
    EditorSignature sign_edit(ReportType::PLAN, this);
    if (sign_edit.exec() == QDialog::Rejected)
    {
        return;
    }
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto report = new LimeReport::ReportEngine(this);
    report->dataManager()->setReportVariable(
        "id_doc", mDocPlanModel->data(mDocPlanModel->index(ui->plan_documents->currentIndex().row(), 0)).toInt());
    report->dataManager()->setReportVariable("signatory_1", settings->value("report_plan/fio_1").toString());
    report->dataManager()->setReportVariable("signatory_2", settings->value("report_plan/fio_2").toString());
    report->dataManager()->setReportVariable("signatory_3", settings->value("report_plan/fio_3").toString());
    report->dataManager()->setReportVariable("signatory_post_1", settings->value("report_plan/post_1").toString());
    report->dataManager()->setReportVariable("signatory_post_2", settings->value("report_plan/post_2").toString());
    report->dataManager()->setReportVariable("signatory_post_3", settings->value("report_plan/post_3").toString());
    report->loadFromFile(":/reports/spravka_on_plan_kcs_kosgu.lrxml");

    QApplication::restoreOverrideCursor();
    report->previewReport();
    QWidget::setEnabled(true);
}

void PlanDocuments::on_button_send_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
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
        popUp->setPopupText("Ни одна справка не выбрана!");
        popUp->show();
        return;
    }
    XmlService xml_service(path_to_save);
    bool success = true;
    const auto selected_rows = ui->plan_documents->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected_rows)
    {
        int id = mDocPlanModel->data(mDocPlanModel->index(index.row(), 0)).toInt();
        if (!xml_service.plan_export(id))
        {
            success = false;
        }
        if (mDocPlanModel->data(mDocPlanModel->index(index.row(), 7)).toString() != "Черновик")
        {
            popUp->setPopupType(Warning);
            popUp->setPopupText("Вы можете отправлять только черновые справки!");
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
                emit PlanDocuments::updateDB();
            });
}

void PlanDocuments::on_button_sln_clicked()
{
    int selectCount = ui->plan_documents->selectionModel()->selectedRows().count();
    if (selectCount != 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите одну справку из списка!");
        popUp->show();
        return;
    }
    int ind_row = ui->plan_documents->selectionModel()->selectedRows().at(0).row();
    int id = mDocPlanModel->data(mDocPlanModel->index(ind_row, 0)).toInt();
    QString number_doc = mDocPlanModel->data(mDocPlanModel->index(ind_row, 1)).toString();
    QString path_to_save = QFileDialog::getSaveFileName(
        0, "Сохранить файл как", QDir::currentPath() + QString("\\Решение_%2_%1.xlsx").arg(number_doc, mCurrentKgrbs),
        "EXCEL(*.xlsx)");
    if (path_to_save == "")
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Файл не был выбран!");
        popUp->show();
        return;
    }
    XmlService xml_service(path_to_save);

    if (xml_service.solution(id))
    {
        popUp->setPopupType(Access);
        popUp->setPopupText("Файл сохранен по пути " + path_to_save);
        popUp->show();
    }
    else
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Ошибка при сохранении. Возможно файл уже используется другой программой.");
        popUp->show();
        return;
    }
}

void PlanDocuments::on_button_merge_clicked()
{
    if (ui->plan_documents->selectionModel()->selectedRows().count() <= 1)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Выберите несколько справок!");
        popUp->show();
        return;
    }
    PlanDocumentEditor create_doc(this);
    if (create_doc.exec() == QDialog::Accepted)
    {
        // Get "Create status"
        QString status_uid;
        QSqlQuery query;
        query.exec(Queries::select_status_plan_create);
        if (query.next())
        {
            status_uid = query.value("uid").toString();
        }
        QSqlDatabase::database().transaction();
        query.prepare(Queries::insert_plan_document);
        query.bindValue(":year_n", mCurrentYear);
        query.bindValue(":kgrbs", mCurrentKgrbs);
        query.bindValue(":doc_number", create_doc.docNumber().isEmpty() ? QVariant() : create_doc.docNumber());
        query.bindValue(":doc_date", create_doc.docDate());
        query.bindValue(":status_uid", status_uid);
        query.bindValue(":basis_uid", create_doc.basisUid().isEmpty() ? QVariant() : create_doc.basisUid());
        query.bindValue(":user_comment", create_doc.userComment().isEmpty() ? QVariant() : create_doc.userComment());
        query.bindValue(":foreign_uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));

        if (!query.exec())
        {
            popUp->setPopupType(Error);
            popUp->setPopupText("Не удалось добавить справку в базу данных!");
            popUp->show();
            QSqlDatabase::database().rollback();
            qCritical() << query.lastError();
            return;
        }
        int new_id = query.lastInsertId().toInt();
        const auto selected_rows = ui->plan_documents->selectionModel()->selectedRows();
        query.prepare(Queries::insert_merge_in_exp_plan);
        query.bindValue(":new_id", new_id);
        for (const QModelIndex &index : selected_rows)
        {
            int old_id = mDocPlanModel->data(mDocPlanModel->index(index.row(), 0)).toInt();
            query.bindValue(":old_id", old_id);
            if (!query.exec())
            {
                popUp->setPopupType(Error);
                popUp->setPopupText("Не удалось добавить данные в справку!");
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
