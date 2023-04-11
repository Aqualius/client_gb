#ifndef MIGRATIONDB_H
#define MIGRATIONDB_H

#include <QSqlDatabase>

class MigrationDB
{
public:
    MigrationDB(const QSqlDatabase &db);

    bool check_db();

    bool execute_sql_script_file(const QString &fileName);

    bool check_migration(const QString &fileName);

private:
    QSqlDatabase db;
};

#endif  // MIGRATIONDB_H
