#include "updater.h"

#include <QColor>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <cstring>

#ifdef Q_OS_WIN32
#include <windows.h>

#include <QCoreApplication>
#include <QSettings>
#endif

#include "sql_queries.h"
#include "zip_file.hpp"

Updater::Updater(QString path) : mPath(path)
{
}

Updater::~Updater()
{
}

bool Updater::update_base()
{
    QSqlDatabase sdb = QSqlDatabase::database();
    sdb.exec("PRAGMA foreign_keys = OFF");
    sdb.transaction();
    miniz_cpp::zip_file file(mPath.toStdString());
    bool check_meta = false;
    for (auto &member : file.infolist())
    {
        QString str = member.filename.c_str();
        bool check_error = false;
        if (str == "meta.json")
        {
            update_meta(file.read(member).c_str());
            check_meta = true;
        }
        else if (str == "management/document_statuses.json")
        {
            check_error = !update_statuses(file.read(member).c_str());
        }
        else if (str == "refs/kubp.json")
        {
            check_error = !update_kubp(file.read(member).c_str());
        }
        else if (str == "refs/kkrb/kgrbs.json")
        {
            check_error = !update_kkrb(file.read(member).c_str(), Queries::delete_kgrbs, Queries::insert_kgrbs);
        }
        else if (str == "refs/kkrb/krp.json")
        {
            check_error = !update_kkrb(file.read(member).c_str(), Queries::delete_krp, Queries::insert_krp);
        }
        else if (str == "refs/kkrb/kcs.json")
        {
            check_error = !update_kkrb(file.read(member).c_str(), Queries::delete_kcs, Queries::insert_kcs);
        }
        else if (str == "refs/kkrb/kvr.json")
        {
            check_error = !update_kkrb(file.read(member).c_str(), Queries::delete_kvr, Queries::insert_kvr);
        }
        else if (str == "refs/kkrb/kosgu.json")
        {
            check_error = !update_kkrb(file.read(member).c_str(), Queries::delete_kosgu, Queries::insert_kosgu);
        }
        else if (str == "expenses/bases.json")
        {
            check_error = !update_bases(file.read(member).c_str());
        }
        else if (str == "expenses/plan.json")
        {
            check_error = !update_plan(file.read(member).c_str());
        }
        else if (str == "expenses/fact.json")
        {
            check_error = !update_fact(file.read(member).c_str());
        }
        if (check_error)
        {
            sdb.rollback();
            QSqlDatabase::database().exec("PRAGMA foreign_keys = ON");
            return false;
        }
    }
    if (!check_meta)
    {
        sdb.rollback();
        QSqlDatabase::database().exec("PRAGMA foreign_keys = ON");
        return false;
    }
    sdb.commit();
    QSqlDatabase::database().exec("PRAGMA foreign_keys = ON");
    return true;
}

bool Updater::update_meta(const QByteArray &data)
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString kgrbs = settings.value("settings/kgrbs").toString();
    QJsonObject jsonObj = QJsonDocument::fromJson(data).object();
    settings.setValue("settings/year", jsonObj["year"].toString());
    const QJsonArray arr = jsonObj["kgrbs"].toArray();
    if (!arr.contains(kgrbs) && !arr.empty())
    {
        settings.setValue("settings/kgrbs", jsonObj["kgrbs"].toArray()[0].toString());
    }
    return true;
}

bool Updater::update_statuses(const QByteArray &data)
{
    QSqlQuery query;
    if (!query.exec(Queries::delete_statuses))
    {
        qCritical() << "Error on delete" << query.lastError();
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject jsonObj = elem.toObject();
        query.prepare(Queries::insert_statuses);
        query.bindValue(":uid", jsonObj["uid"].toString());
        query.bindValue(":documentType", jsonObj["documentType"].toString());
        query.bindValue(":title", jsonObj["title"].toString());
        query.bindValue(":statusType", jsonObj["statusType"].toString());
        query.bindValue(":priority", jsonObj["priority"].toInt());
        query.bindValue(":color", jsonObj["color"].toString());
        if (!query.exec())
        {
            qCritical() << "Error on insert" << query.lastError();

            return false;
        }
    }
    return true;
}

bool Updater::update_kubp(const QByteArray &data)
{
    QSqlQuery query;
    if (!query.exec(Queries::delete_kubp))
    {
        qCritical() << "Error on delete" << query.lastError();

        return false;
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject jsonObj = elem.toObject();
        query.prepare(Queries::insert_kubp);
        query.bindValue(":code", jsonObj["code"].toString());
        query.bindValue(":kgrbs", jsonObj["kgrbs"].toString());
        query.bindValue(":title", jsonObj["title"].toString());
        query.bindValue(":deleted", jsonObj["deleted"].toBool());
        if (!query.exec())
        {
            qCritical() << "Error on insert" << query.lastError();

            return false;
        }
    }
    return true;
}

bool Updater::update_kkrb(const QByteArray &data, const QString &del, const QString &ins)
{
    QSqlQuery query;
    if (!query.exec(del))
    {
        qCritical() << "Error on delete: " << query.lastError();

        return false;
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject jsonObj = elem.toObject();
        query.prepare(ins);
        query.bindValue(":code", jsonObj["code"].toString());
        query.bindValue(":title", jsonObj["title"].toString());
        query.bindValue(":deleted", jsonObj["deleted"].toBool());
        query.bindValue(":notes_req", jsonObj["notesRequired"].toBool());
        if (!query.exec())
        {
            qCritical() << "Error on insert: " << query.lastError();

            return false;
        }
    }
    return true;
}

// bool Updater::update_kosgu(const QByteArray &data, const QString &del, const QString &ins)
//{
//    QSqlQuery query;
//    if (!query.exec(del))
//    {
//        qCritical() << "Error on delete: " << query.lastError();

//        return false;
//    }
//    const QJsonArray arr = QJsonDocument::fromJson(data).array();
//    for (const auto &elem : arr)
//    {
//        QJsonObject jsonObj = elem.toObject();
//        query.prepare(ins);
//        query.bindValue(":code", jsonObj["code"].toString());
//        query.bindValue(":title", jsonObj["title"].toString());
//        query.bindValue(":deleted", jsonObj["deleted"].toBool());
//        query.bindValue(":deleted", jsonObj["deleted"].toBool());
//        if (!query.exec())
//        {
//            qCritical() << "Error on insert: " << query.lastError();

//            return false;
//        }
//    }
//    return true;
//}

bool Updater::update_bases(const QByteArray &data)
{
    QSqlQuery query;
    if (!query.exec(Queries::delete_bases) || !query.exec(Queries::delete_bases_scope))
    {
        qCritical() << "Error on delete: " << query.lastError();

        return false;
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject jsonObj = elem.toObject();
        query.prepare(Queries::insert_bases);
        query.bindValue(":uid", jsonObj["uid"].toString());
        query.bindValue(":basis_type", jsonObj["type"].toString());
        query.bindValue(":basis_number", jsonObj["number"].toString());
        query.bindValue(":basis_date", jsonObj["date"].toString());
        query.bindValue(":with_changes", jsonObj["withChanges"].toBool());
        if (!query.exec())
        {
            qCritical() << "Error on insert: " << query.lastError();

            return false;
        }
        query.prepare(Queries::insert_bases_scope);
        query.bindValue(":uid", jsonObj["uid"].toString());
        const QJsonArray scope = jsonObj["scope"].toArray();
        for (const auto &el : scope)
        {
            QJsonObject obj = el.toObject();
            query.bindValue(":docNumber", obj["docNumber"].toInt());
            query.bindValue(":kgrbs", obj["kgrbs"].toString());

            if (!query.exec())
            {
                qCritical() << "Error on insert: " << query.lastError();
                return false;
            }
        }
    }
    return true;
}

bool Updater::update_plan(const QByteArray &data)
{
    QSqlQuery query;
    if (!query.exec(Queries::delete_plan_doc))
    {
        qCritical() << "Error on delete doc: " << query.lastError();

        return false;
    }
    if (!query.exec(Queries::delete_plan_data))
    {
        qCritical() << "Error on delete data: " << query.lastError();

        return false;
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject obj = elem.toObject();
        if (!obj["foreignUid"].toString().isEmpty())
        {
            query.prepare(Queries::delete_plan_data_by_foreign_uid);
            query.bindValue(":uid", obj["foreignUid"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on delete plan data: " << query.lastError();
                return false;
            }
            query.prepare(Queries::delete_plan_doc_by_foreign_uid);
            query.bindValue(":uid", obj["foreignUid"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on delete plan doc: " << query.lastError();
                return false;
            }
        }
        query.prepare(Queries::import_document_plan);
        query.bindValue(":year", obj["year"].toInt());
        query.bindValue(":kgrbs", obj["kgrbs"].toString());
        query.bindValue(":changes", obj["changes"].toBool());
        query.bindValue(":number", obj["number"].toInt());
        query.bindValue(":date", obj["date"].toString());
        query.bindValue(":statusUid", obj["statusUid"].toString());
        query.bindValue(":basisUid", obj["basisUid"].toString());
        query.bindValue(":comment", obj["comment"].toString().isEmpty() ? QVariant() : obj["comment"].toString());
        query.bindValue(":foreignUid",
                        obj["foreignUid"].toString().isEmpty() ? QVariant() : obj["foreignUid"].toString());
        if (!query.exec())
        {
            qCritical() << "Error on insert doc: " << query.lastError();

            return false;
        }
        int doc_id = query.lastInsertId().toInt();
        query.prepare(Queries::import_data_plan);
        const QJsonArray data = obj.find("data")->toArray();
        for (const auto &elem_data : data)
        {
            QJsonObject tmp_obj = elem_data.toObject();
            query.bindValue(":document_id", doc_id);
            query.bindValue(":month", tmp_obj["month"].toInt());
            query.bindValue(":amount", tmp_obj["amount"].toDouble());
            query.bindValue(":krp", tmp_obj["krp"].toString());
            query.bindValue(":kcs", tmp_obj["kcs"].toString());
            query.bindValue(":kvr", tmp_obj["kvr"].toString());
            query.bindValue(":kosgu", tmp_obj["kosgu"].toString());
            query.bindValue(":kubp", tmp_obj["kubp"].toString());
            query.bindValue(":expensesPart", tmp_obj["expensesPart"].toString());
            query.bindValue(":notes", tmp_obj["notes"].toString().isEmpty() ? QVariant() : tmp_obj["notes"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on insert data: " << query.lastError();

                return false;
            }
        }
    }
    return true;
}

bool Updater::update_fact(const QByteArray &data)
{
    QSqlQuery query;
    if (!query.exec(Queries::delete_fact_doc))
    {
        qCritical() << "Error on delete doc: " << query.lastError();

        return false;
    }
    if (!query.exec(Queries::delete_fact_data))
    {
        qCritical() << "Error on delete data: " << query.lastError();

        return false;
    }
    const QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto &elem : arr)
    {
        QJsonObject obj = elem.toObject();
        if (!obj["foreignUid"].toString().isEmpty())
        {
            query.prepare(Queries::delete_fact_data_by_foreign_uid);
            query.bindValue(":uid", obj["foreignUid"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on delete fact data: " << query.lastError();
                return false;
            }
            query.prepare(Queries::delete_fact_doc_by_foreign_uid);
            query.bindValue(":uid", obj["foreignUid"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on delete fact doc: " << query.lastError();
                return false;
            }
        }
        query.prepare(Queries::import_document_fact);
        query.bindValue(":year", obj["year"].toInt());
        query.bindValue(":kgrbs", obj["kgrbs"].toString());
        query.bindValue(":number", obj["number"].toInt());
        query.bindValue(":date", obj["date"].toString());
        query.bindValue(":fundedAt", obj["fundedAt"].toString());
        query.bindValue(":paymentType", obj["paymentType"].toString());
        query.bindValue(":statusUid", obj["statusUid"].toString());
        query.bindValue(":foreignUid",
                        obj["foreignUid"].toString().isEmpty() ? QVariant() : obj["foreignUid"].toString());
        query.bindValue(":comment", obj["comment"].toString().isEmpty() ? QVariant() : obj["comment"].toString());
        if (!query.exec())
        {
            qCritical() << "Error on insert doc: " << query.lastError();

            return false;
        }
        int doc_id = query.lastInsertId().toInt();
        query.prepare(Queries::import_data_fact);
        const QJsonArray data = obj.find("data")->toArray();
        for (const auto &elem_data : data)
        {
            QJsonObject tmp_obj = elem_data.toObject();
            query.bindValue(":document_id", doc_id);
            query.bindValue(":amount", tmp_obj["amount"].toDouble());
            query.bindValue(":krp", tmp_obj["krp"].toString());
            query.bindValue(":kcs", tmp_obj["kcs"].toString());
            query.bindValue(":kvr", tmp_obj["kvr"].toString());
            query.bindValue(":kosgu", tmp_obj["kosgu"].toString());
            query.bindValue(":kubp", tmp_obj["kubp"].toString());
            query.bindValue(":expensesPart", tmp_obj["expensesPart"].toString());
            query.bindValue(":notes", tmp_obj["notes"].toString().isEmpty() ? QVariant() : tmp_obj["notes"].toString());
            if (!query.exec())
            {
                qCritical() << "Error on insert data: " << query.lastError();

                return false;
            }
        }
    }
    return true;
}

void Updater::runProgramUpdater()
{
    QProcess process;

#ifdef Q_OS_WIN32
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NEW_CONSOLE;
        args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
    });
#endif
    process.setProgram("./rustyupdater.exe");
    process.setArguments({ "--wait-for", QString::number(QCoreApplication::applicationPid()), "--then-run",
                           QCoreApplication::applicationFilePath() });
    process.startDetached();
}

QProcess *Updater::createProgramUpdateChecker(QObject *parent)
{
    auto process = new QProcess(parent);
    process->setProgram("./rustyupdater.exe");
    process->setArguments({ "check" });
    return process;
}
