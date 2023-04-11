#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QLibraryInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTranslator>

#include "mainwindow.h"
#include "migration_db.h"

// Умный указатель на файл логирования
QScopedPointer<QFile> m_logFile;

// Объявление обработчика
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qSetMessagePattern("[%{time h:mm:ss}] [%{type}] %{function}: %{message}");

    // Устанавливаем файл логирования
    m_logFile.reset(new QFile("./log.txt"));
    // Открываем файл логирования
    m_logFile.data()->open(QFile::Append | QFile::Text);
    // Устанавливаем обработчик
    qInstallMessageHandler(messageHandler);

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        a.installTranslator(&qtTranslator);
    }

    QTranslator qtBaseTranslator;
    if (qtBaseTranslator.load("qtbase_" + QLocale::system().name(),
                              QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        a.installTranslator(&qtBaseTranslator);
    }

    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName("budget.db");
    if (!sdb.open())
    {
        qDebug() << sdb.lastError().text();
        return 1;
    }
    else
    {
        sdb.exec("PRAGMA foreign_keys = ON");
    }

    MigrationDB migration(sdb);
    if (!migration.check_db())
    {
        qCritical() << "error migration db";
    }

    MainWindow w;
    w.showMaximized();
    qDebug() << "Программа начала свою работу!";
    return a.exec();
}

// Реализация обработчика
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Открываем поток записи в файл
    QTextStream out(m_logFile.data());
    // Записываем дату записи
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // По типу определяем, к какому уровню относится сообщение
    switch (type)
    {
    case QtInfoMsg:
        out << "INF ";
        break;
    case QtDebugMsg:
        out << "DBG ";
        break;
    case QtWarningMsg:
        out << "WRN ";
        break;
    case QtCriticalMsg:
        out << "CRT ";
        break;
    case QtFatalMsg:
        out << "FTL ";
        break;
    }
    // Записываем в вывод сообщение
    out << msg << Qt::endl;
    out.flush();  // Очищаем буферизированные данные
}
