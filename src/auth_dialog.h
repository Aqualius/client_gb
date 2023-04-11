#ifndef AUTH_DIALOG_H
#define AUTH_DIALOG_H

#include <QDialog>
#include <QSettings>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "request_manager.h"

namespace Ui
{
class AuthDialog;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(AuthType auth_type = AuthType::Standart, QWidget *parent = nullptr);
    ~AuthDialog();

    QString login() const;

    QString password() const;

    QString token();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::AuthDialog *ui;
    QSettings *settings;
};

#endif  // AUTH_DIALOG_H
