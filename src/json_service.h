#ifndef JSONSERVICE_H
#define JSONSERVICE_H

#include <QFileDialog>

class JsonService
{
public:
    JsonService();

    void plan_import(const QString &file_name);
};

#endif  // JSONSERVICE_H
