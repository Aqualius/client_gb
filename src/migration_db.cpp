#include "migration_db.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QTime>

#include "sql_queries.h"

MigrationDB::MigrationDB(const QSqlDatabase &db) : db(db)
{
}

bool MigrationDB::check_db()
{
    QSqlQuery query;
    if (!query.exec(Queries::create_table_migration))
    {
        return false;
    }

    db.exec("PRAGMA foreign_keys = OFF");
    db.transaction();
    const QFileInfoList migrationsList =
        QDir(":/migrations").entryInfoList({ "*.sql" }, QDir::Filter::Files, QDir::SortFlag::Name);

    for (const QFileInfo &file : migrationsList)
    {
        if (check_migration(file.fileName()))
        {
            continue;
        }
        if (!execute_sql_script_file(file.absoluteFilePath()))
        {
            db.rollback();
            db.exec("PRAGMA foreign_keys = ON");
            return false;
        }
        query.prepare(Queries::insert_migration);
        query.bindValue(":script", file.fileName());
        query.bindValue(":installed_on", QString("%1 %2").arg(QDate::currentDate().toString("yyyy-MM-dd"),
                                                              QTime::currentTime().toString()));
        if (!query.exec())
        {
            db.rollback();
            db.exec("PRAGMA foreign_keys = ON");
            return false;
        }
    }

    db.commit();
    db.exec("PRAGMA foreign_keys = ON");
    return true;
}

bool MigrationDB::execute_sql_script_file(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.errorString();
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString sql = in.readAll();
    const QStringList statements = sql.split(';', Qt::SkipEmptyParts);

    for (const QString &statement : statements)
    {
        if (statement.trimmed() != "")
        {
            QSqlQuery query(db);
            if (!query.exec(statement))
            {
                qCritical() << "Failed:" << statement << "\nReason:" << query.lastError();
                return false;
            }
        }
    }
    return true;
}

bool MigrationDB::check_migration(const QString &fileName)
{
    QSqlQuery query;
    query.prepare(Queries::check_migration);
    query.bindValue(":script", fileName);
    query.exec();
    return query.next();
}
