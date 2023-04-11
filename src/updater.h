#ifndef UPDATER_H
#define UPDATER_H

#include <QProcess>
#include <QString>

class Updater
{
public:
    Updater(QString path);
    ~Updater();

    bool update_base();

private:
    bool update_meta(const QByteArray &data);

    bool update_statuses(const QByteArray &data);

    bool update_kubp(const QByteArray &data);

    bool update_kkrb(const QByteArray &data, const QString &del, const QString &ins);

    bool update_bases(const QByteArray &data);

    bool update_plan(const QByteArray &data);

    bool update_fact(const QByteArray &data);

public:
    static void runProgramUpdater();

    static QProcess *createProgramUpdateChecker(QObject *parent);

public slots:

private:
    QString mPath;
};

#endif  // UPDATER_H
