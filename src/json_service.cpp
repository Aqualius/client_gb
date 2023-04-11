#include "json_service.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>

#include "sql_queries.h"

JsonService::JsonService()
{
}

void JsonService::plan_import(const QString &file_name)
{
    // Создаём объект файла и открываем его на чтение
    QFile json_file(file_name);
    if (!json_file.open(QIODevice::ReadOnly))
    {
        return;
    }
    // Считываем весь файл
    QByteArray save_data = json_file.readAll();
    // Создаём QJsonDocument
    QJsonDocument json_doc(QJsonDocument::fromJson(save_data));

    QSqlDatabase db = QSqlDatabase::database();
    if (db.transaction())
    {
        bool check_error = false;
        for (int i = 0; i < json_doc.array().size(); i++)
        {
            QSqlQuery query(db);
            query.prepare(Queries::import_document_plan);
            QJsonObject json_obj = json_doc.array()[i].toObject();
            query.bindValue(":year", json_obj["year"].toInt());
            query.bindValue(":kgrbs", json_obj["kgrbs"].toString());
            query.bindValue(":changes", json_obj["changes"].toBool());
            query.bindValue(":number",
                            json_obj["number"].toString().isEmpty() ? QVariant() : json_obj["number"].toString());
            query.bindValue(":date", json_obj["date"].toString());
            query.bindValue(":statusUid", json_obj["statusUid"].toString());
            query.bindValue(":basisUid",
                            json_obj["basisUid"].toString().isEmpty() ? QVariant() : json_obj["basisUid"].toString());
            query.bindValue(":comment",
                            json_obj["comment"].toString().isEmpty() ? QVariant() : json_obj["comment"].toString());
            query.bindValue(":foreignUid", json_obj["foreignUid"].toString());
            if (!query.exec())
            {
                qCritical() << query.lastError();
                check_error = true;
                break;
            }
            int doc_id = query.lastInsertId().toInt();
            query.prepare(Queries::import_data_plan);
            QJsonArray data = json_obj.find("data")->toArray();
            for (int j = 0; j < data.size(); j++)
            {
                QJsonObject tmp_obj = data[j].toObject();
                query.bindValue(":document_id", doc_id);
                query.bindValue(":month", tmp_obj["month"].toInt());
                query.bindValue(":amount", tmp_obj["amount"].toDouble());
                query.bindValue(":krp", tmp_obj["krp"].toString());
                query.bindValue(":kcs", tmp_obj["kcs"].toString());
                query.bindValue(":kvr", tmp_obj["kvr"].toString());
                query.bindValue(":kosgu", tmp_obj["kosgu"].toString());
                query.bindValue(":kubp", tmp_obj["kubp"].toString());
                query.bindValue(":expensesPart", tmp_obj["expensesPart"].toString());
                query.bindValue(":notes",
                                tmp_obj["notes"].toString().isEmpty() ? QVariant() : tmp_obj["notes"].toString());
                if (!query.exec())
                {
                    qCritical() << query.lastError();
                    db.rollback();
                    check_error = true;
                    break;
                }
            }
            if (check_error)
            {
                break;
            }
        }
        if (!db.commit() || check_error)
        {
            qCritical() << "Failed to commit";
            db.rollback();
        }
    }
    else
    {
        qCritical() << "Failed to start transaction mode";
    }
}
