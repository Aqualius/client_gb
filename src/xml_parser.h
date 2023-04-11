#ifndef XMLSERVICE_H
#define XMLSERVICE_H

#include <QDomElement>
#include <QFile>
#include <QIODevice>
#include <QListView>
#include <QSqlQuery>

#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxchartsheet.h"
#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

class XmlService
{
public:
    XmlService(const QString &path);

    bool plan_export(int doc_id);

    bool fact_export(int doc_id);

    bool valid_xml(const QString &path_schema);

    bool plan_import(const QString &path);

    bool fact_import(const QString &path);

    void add_node_with_text(QDomDocument &doc, QDomElement &root, const QString &name, const QVariant &data);

    void add_nodes(QDomDocument &doc, QDomElement &root, const QString &name, QSqlQuery &query);

    QString lastError() const;

    bool solution(int id);

    bool checkForeignUid(const QString &uid, bool plan = true);

private:
    QString mPath;
    QString mLastError;
    QString mKgrbs;
    QString mUid;

private:
    QList<QVariant> *list;
};

#endif  // XMLSERVICE_H
