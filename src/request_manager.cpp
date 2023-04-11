#include "request_manager.h"

#include <QDataStream>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

RequestManager::RequestManager(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &RequestManager::replyFinished);
    connect(this, &RequestManager::startUpdate, this, &RequestManager::get_update);
}

void RequestManager::token(const QString &login, const QString &password)
{
    mReqType = RequestType::GetToken;
    QNetworkRequest request(QUrl("http://budgetiq.mf.io/api/user/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["username"] = login;
    obj["password"] = password;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    manager->post(request, data);
}

void RequestManager::validate_token(const QString &token)
{
    mReqType = RequestType::ValidateToken;
    QNetworkRequest request(QUrl("http://budgetiq.mf.io/api/user"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toLocal8Bit());

    manager->get(request);
}

void RequestManager::get_update()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString token = settings.value("update/token").toString();
    mReqType = RequestType::UpdateDB;
    QNetworkRequest request(QUrl("http://budgetiq.mf.io/api/external/client-gb/update.zip?year=2022"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toLocal8Bit());

    manager->get(request);
}
void RequestManager::replyFinished(QNetworkReply *reply)
{
    int code;
    switch (mReqType)
    {
    case RequestType::EmptyRequest:
        return;
    case RequestType::UpdateDB:
        code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        mReqType = RequestType::EmptyRequest;
        if (code == 200)
        {
            QByteArray b = reply->readAll();
            QFile file(QDir::currentPath() + "/update/cgb_update.zip");
            file.open(QIODevice::WriteOnly);
            file.write(b);
            file.close();
            emit updateLocal(QDir::currentPath() + "/update/cgb_update.zip");
        }
        else
        {
            mLastError = "Что-то пошло не так!";
        }
        reply->deleteLater();
        break;
    case RequestType::GetToken:
        code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        mReqType = RequestType::EmptyRequest;
        if (code == 200)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QSettings settings("settings.ini", QSettings::IniFormat);
            settings.setValue("update/token", findKey("token", doc.object()).toString());
            emit startUpdate();
        }
        reply->deleteLater();
        if (code == 400)
        {
            mLastError = "Поля должны быть заполнены!";
            emit authToken(AuthType::EmptyField);
        }
        else if (code == 401)
        {
            mLastError = "Введены неверные данные!";
            emit authToken(AuthType::NotCorrectPassword);
        }
        else if (code == 404)
        {
            mLastError = "Такого пользователя нет!";
            emit authToken(AuthType::NotFoundUser);
        }
        break;
    case RequestType::ValidateToken:
        code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        mReqType = RequestType::EmptyRequest;
        if (code == 200)
        {
            emit startUpdate();
        }
        reply->deleteLater();
        if (code == 400)
        {
            mLastError = "Поля должны быть заполнены!";
            emit authToken(AuthType::EmptyField);
        }
        else if (code == 401)
        {
            mLastError = "Токен не верный!";
            emit authToken(AuthType::Standart);
        }
        else if (code == 404)
        {
            mLastError = "Такого пользователя нет!";
            emit authToken(AuthType::NotFoundUser);
        }
        break;
    }
}

QJsonValue RequestManager::findKey(const QString &key, const QJsonValue &value)
{
    if (value.isObject())
    {
        const QJsonObject obj = value.toObject();
        if (obj.contains(key))
            return obj.value(key);

        for (const auto &value : obj)
        {
            QJsonValue recurse = findKey(key, value);
            if (!recurse.isNull())
                return recurse;
        }
    }
    else if (value.isArray())
    {
        const QJsonArray arr = value.toArray();
        for (const auto &value : arr)
        {
            QJsonValue recurse = findKey(key, value);
            if (!recurse.isNull())
                return recurse;
        }
    }

    return QJsonValue();
}

QString RequestManager::lastError() const
{
    return mLastError;
}
