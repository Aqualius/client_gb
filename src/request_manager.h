#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

enum AuthType
{
    Standart,
    NotFoundUser,
    NotCorrectPassword,
    EmptyField
};

enum RequestType
{
    EmptyRequest,
    ValidateToken,
    GetToken,
    UpdateDB
};

class RequestManager : public QObject
{
    Q_OBJECT
public:
    RequestManager(QObject *parent = 0);

    void token(const QString &login, const QString &password);

    QJsonValue findKey(const QString &key, const QJsonValue &value);

    void validate_token(const QString &token);

    void get_update();

    QString lastError() const;

public slots:
    void replyFinished(QNetworkReply *reply);

signals:
    void authToken(AuthType authType);

    void startUpdate();

    void updateLocal(const QString &path);

private:
    QNetworkAccessManager *manager;
    QString mLastError;
    RequestType mReqType;
};

#endif  // REQUESTMANAGER_H
