#include "xml_parser.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QSqlError>
#include <QSqlRecord>
#include <QTextStream>
#include <QUuid>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

#include "sql_queries.h"

XmlService::XmlService(const QString &path) : mPath(path)
{
}

bool XmlService::plan_export(int doc_id)
{
    //// Get number document ////
    QString number_doc;
    QSqlQuery query;
    query.prepare(Queries::num_from_plan_doc);
    query.bindValue(":id", doc_id);
    query.exec();
    if (query.next())
    {
        number_doc = query.value(0).toString();
    }
    else
    {
        mLastError = "Не удалось получить номер файла";
        return false;
    }
    /////////////////////////////

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8")"));
    QDomElement root = doc.createElement("expenses_plan_document");
    doc.appendChild(root);
    query.prepare(Queries::export_document_plan);
    query.bindValue(":id", doc_id);
    query.exec();
    QDomElement data;
    QDomText text;
    add_nodes(doc, root, "", query);

    data = doc.createElement("data");
    root.appendChild(data);
    query.prepare(Queries::export_data_plan);
    query.bindValue(":id", doc_id);
    query.exec();
    add_nodes(doc, data, "item", query);
    QString file_name;
    if (mPath.indexOf(".xml") == -1)
    {
        file_name = QString("/ex_plan_doc_%2_%1_%3.xml").arg(number_doc, mKgrbs, mUid);
    }
    QFile file(mPath + file_name);
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream in(&file);
        doc.save(in, 4);
        file.flush();
        file.close();
    }
    else
    {
        mLastError = QString("Ошибка открытия файла " + mPath);
        return false;
    }
    return true;
}

bool XmlService::fact_export(int doc_id)
{
    //// Get number document ////
    QString number_doc;
    QSqlQuery query;
    query.prepare(Queries::num_from_fact_doc);
    query.bindValue(":id", doc_id);
    query.exec();
    if (query.next())
    {
        number_doc = query.value(0).toString();
    }
    else
    {
        mLastError = "Не удалось получить номер файла";
        return false;
    }
    /////////////////////////////

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8")"));
    QDomElement root = doc.createElement("expenses_fact_document");
    doc.appendChild(root);
    query.prepare(Queries::export_document_fact);
    query.bindValue(":id", doc_id);
    query.exec();
    QDomElement data;
    QDomText text;
    add_nodes(doc, root, "", query);

    data = doc.createElement("data");
    root.appendChild(data);
    query.prepare(Queries::export_data_fact);
    query.bindValue(":id", doc_id);
    query.exec();
    add_nodes(doc, data, "item", query);
    QString file_name;
    if (mPath.indexOf(".xml") == -1)
    {
        file_name = QString("/ex_fact_doc_%2_%1_%3.xml").arg(number_doc, mKgrbs, mUid);
    }
    QFile file(mPath + file_name);
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream in(&file);
        doc.save(in, 4);
        file.flush();
        file.close();
    }
    else
    {
        mLastError = QString("Ошибка сохранения файла: " + query.lastError().text());
        return false;
    }
    return true;
}

bool XmlService::valid_xml(const QString &path_schema)
{
    bool flag = false;
    QFile xsdfile(path_schema);
    if (!xsdfile.open(QIODevice::ReadOnly))
    {
        flag = false;
    }
    QXmlSchema schema;
    if (schema.load(&xsdfile, QUrl::fromLocalFile(xsdfile.fileName())))
    {
        if (schema.isValid())
        {
            QFile xml_file(mPath);
            if (!xml_file.exists() || !xml_file.open(QIODevice::ReadOnly | QFile::Text))
            {
                mLastError = QString("Ошибка открытия файла " + mPath);
                return false;
            }
            QXmlSchemaValidator xmlvalidator(schema);
            if (xmlvalidator.validate(&xml_file, QUrl::fromLocalFile(xml_file.fileName())))
            {
                flag = true;
            }
        }
    }
    return flag;
}

bool XmlService::plan_import(const QString &path)
{
    mPath = path;
    QFile xml_file(mPath);
    if (!xml_file.exists() || !xml_file.open(QIODevice::ReadOnly | QFile::Text))
    {
        mLastError = QString("Не удалось открыть файл.");
        return false;
    }
    if (!valid_xml(":/schemas/plan_doc.xsd"))
    {
        mLastError = QString("Ошибка структуры файла.");
        return false;
    }
    QDomDocument document;
    document.setContent(&xml_file);
    QDomElement top_element = document.documentElement();
    QDomNode node = top_element.firstChild();

    QSqlDatabase sdb = QSqlDatabase::database();
    QSqlQuery query;

    // Get "Create status"
    QString status_uid;
    query.exec(Queries::select_status_plan_create);
    if (query.next())
    {
        status_uid = query.value("uid").toString();
    }

    sdb.transaction();
    query.prepare(Queries::import_document_plan);
    while (!node.isNull())
    {
        QDomElement elem = node.toElement();
        if (elem.isNull())
        {
            node = node.nextSibling();
            continue;
        }
        if (elem.tagName() == "foreignUid")
        {
            QString uid = elem.text();
            if (!uid.isEmpty())
            {
                if (checkForeignUid(elem.text()))
                {
                    uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
                }
            }
            query.bindValue(":foreignUid", uid.isEmpty() ? QVariant() : uid);
        }
        else if (elem.tagName() == "data")
        {
            query.bindValue(":statusUid", status_uid);
            if (!query.exec())
            {
                qDebug() << query.lastQuery();
                mLastError = QString("Ошибка добавления документа: " + query.lastError().text());
                sdb.rollback();
                return false;
            }
            QDomNode elem_node = elem.firstChild();
            int doc_id = query.lastInsertId().toInt();
            while (!elem_node.isNull())
            {
                QString str;
                QSqlQuery q;
                q.prepare(Queries::import_data_plan);
                q.bindValue(":document_id", doc_id);
                QDomNode tmp_node = elem_node.firstChild();
                while (!tmp_node.isNull())
                {
                    QDomElement tmp_elem = tmp_node.toElement();
                    if (!tmp_elem.isNull())
                    {
                        str += tmp_elem.text() + " ";
                        q.bindValue(":" + tmp_elem.tagName(), tmp_elem.text().isEmpty() ? QVariant() : tmp_elem.text());
                    }
                    tmp_node = tmp_node.nextSibling();
                }
                if (!q.exec())
                {
                    qCritical() << "В импорте планов эта строка не затянулась" << str;
                    mLastError = QString("Ошибка добавления данных документа: " + q.lastError().text());
                    sdb.rollback();
                    return false;
                }
                elem_node = elem_node.nextSibling();
            }
        }
        else
        {
            query.bindValue(":" + elem.tagName(), elem.text().isEmpty() ? QVariant() : elem.text());
        }
        node = node.nextSibling();
    }
    sdb.commit();
    return true;
}

bool XmlService::fact_import(const QString &path)
{
    mPath = path;
    QFile xml_file(mPath);
    if (!xml_file.exists() || !xml_file.open(QIODevice::ReadOnly | QFile::Text))
    {
        mLastError = QString("Не удалось открыть файл.");
        return false;
    }
    if (!valid_xml(":/schemas/fact_doc.xsd"))
    {
        mLastError = QString("Ошибка структуры файла.");
        return false;
    }
    QDomDocument document;
    document.setContent(&xml_file);
    QDomElement top_element = document.documentElement();
    QDomNode node = top_element.firstChild();

    QSqlDatabase sdb = QSqlDatabase::database();
    QSqlQuery query;

    // Get "Create status"
    QString status_uid = "";
    query.exec(Queries::select_status_fact_create);
    if (query.next())
    {
        status_uid = query.value("uid").toString();
    }

    query.prepare(Queries::import_document_fact);
    while (!node.isNull())
    {
        QDomElement elem = node.toElement();
        if (elem.isNull())
        {
            node = node.nextSibling();
            continue;
        }
        if (elem.tagName() == "foreignUid")
        {
            QString uid = elem.text();
            if (!uid.isEmpty())
            {
                if (checkForeignUid(elem.text(), false))
                {
                    uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
                }
            }
            query.bindValue(":foreignUid", uid.isEmpty() ? QVariant() : uid);
        }
        else if (elem.tagName() == "data")
        {
            query.bindValue(":statusUid", status_uid);
            if (!query.exec())
            {
                mLastError = QString("Ошибка добавления документа: " + query.lastError().text());
                sdb.rollback();
                return false;
            }
            ///// START TRANSACTION /////
            sdb.transaction();
            QDomNode elem_node = elem.firstChild();
            int doc_id = query.lastInsertId().toInt();
            while (!elem_node.isNull())
            {
                QString str;
                QSqlQuery q;
                q.prepare(Queries::import_data_fact);
                q.bindValue(":document_id", doc_id);
                QDomNode tmp_node = elem_node.firstChild();
                while (!tmp_node.isNull())
                {
                    QDomElement tmp_elem = tmp_node.toElement();
                    if (!tmp_elem.isNull())
                    {
                        str += tmp_elem.text() + " ";
                        q.bindValue(":" + tmp_elem.tagName(), tmp_elem.text().isEmpty() ? QVariant() : tmp_elem.text());
                    }
                    tmp_node = tmp_node.nextSibling();
                }
                if (!q.exec())
                {
                    qCritical() << "В импорте планов эта строка не затянулась" << str;
                    mLastError = QString("Ошибка добавления данных документа: " + q.lastError().text());
                    sdb.rollback();
                    return false;
                }
                elem_node = elem_node.nextSibling();
            }
            ///// END TRANSACTION /////
            sdb.commit();
        }
        else
        {
            query.bindValue(":" + elem.tagName(), elem.text().isEmpty() ? QVariant() : elem.text());
        }
        node = node.nextSibling();
    }
    return true;
}

void XmlService::add_node_with_text(QDomDocument &doc, QDomElement &root, const QString &name, const QVariant &data)
{
    QDomElement tmp_data = doc.createElement(name);
    QDomText text = doc.createTextNode(data.toString());
    tmp_data.appendChild(text);
    root.appendChild(tmp_data);
}

void XmlService::add_nodes(QDomDocument &doc, QDomElement &root, const QString &name, QSqlQuery &query)
{
    QDomElement data;
    QDomText text;
    while (query.next())
    {
        QDomElement elem = name == "" ? root : doc.createElement(name);
        for (int i = 0; i < query.record().count(); i++)
        {
            if (query.value(i).isNull())
            {
                continue;
            }
            if (query.record().fieldName(i) == "amount")
            {
                add_node_with_text(doc, elem, query.record().fieldName(i),
                                   QString::number(query.value(i).toDouble(), 'f', 2));
                continue;
            }
            else if (query.record().fieldName(i) == "kgrbs")
            {
                mKgrbs = query.value(i).toString();
            }
            else if (query.record().fieldName(i) == "foreignUid")
            {
                mUid = query.value(i).toString().split("-")[0];
            }
            add_node_with_text(doc, elem, query.record().fieldName(i), query.value(i));
        }
        if (name != "")
        {
            root.appendChild(elem);
        }
    }
}

QString XmlService::lastError() const
{
    return mLastError;
}

bool XmlService::solution(int id)
{
    Document file(":/template/solution_plan.xlsx");
    QSqlDatabase sdb = QSqlDatabase::database();
    QSqlQuery query;

    const int style = 2;
    const int row = 2;
    int num_row = 0;

    ///// I раздел ///////

    query.prepare(Queries::solution_plan_without_kosgu);
    query.bindValue(":id", id);

    query.exec();
    while (query.next())
    {
        QString kgrbs = query.value("kgrbs").toString();
        QString krp = query.value("krp").toString();
        QString kcs = query.value("kcs").toString();
        QString kvr = query.value("kvr").toString();
        double amount = query.value("amount").toDouble() / 1000;

        QString tmp_row = QString::number(row + num_row);
        for (int i = 1; i <= 5; i++)
        {
            Format format = file.cellAt(style, i)->format();
            file.write(row + num_row, i, QVariant(), format);
        }
        file.write("A" + tmp_row, kgrbs);
        file.write("B" + tmp_row, krp);
        file.write("C" + tmp_row, kcs);
        file.write("D" + tmp_row, kvr);
        file.write("E" + tmp_row, amount);
        num_row++;
    }

    ///// II раздел ///////

    file.selectSheet("Раздел II");
    num_row = 0;

    query.prepare(Queries::solution_plan_with_kosgu);
    query.bindValue(":id", id);

    query.exec();
    while (query.next())
    {
        QString kgrbs = query.value("kgrbs").toString();
        QString krp = query.value("krp").toString();
        QString kcs = query.value("kcs").toString();
        QString kvr = query.value("kvr").toString();
        QString kosgu = query.value("kosgu").toString();
        double amount = query.value("amount").toDouble() / 1000;

        QString tmp_row = QString::number(row + num_row);
        for (int i = 1; i <= 6; i++)
        {
            Format format = file.cellAt(style, i)->format();
            file.write(row + num_row, i, QVariant(), format);
        }
        file.write("A" + tmp_row, kgrbs);
        file.write("B" + tmp_row, krp);
        file.write("C" + tmp_row, kcs);
        file.write("D" + tmp_row, kvr);
        file.write("E" + tmp_row, kosgu);
        file.write("F" + tmp_row, amount);
        num_row++;
    }
    file.selectSheet("Раздел I");
    return file.saveAs(mPath);
}

bool XmlService::checkForeignUid(const QString &uid, bool plan)
{
    QSqlQuery query;
    QString sql;
    if (plan)
    {
        sql = Queries::check_plan_by_foreign_uid;
    }
    else
    {
        sql = Queries::check_fact_by_foreign_uid;
    }
    query.prepare(sql);
    query.bindValue(":uid", uid);
    query.exec();
    if (query.next())
    {
        return true;
    }
    return false;
}
