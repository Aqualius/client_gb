#include "auth_dialog.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "ui_auth_dialog.h"

AuthDialog::AuthDialog(AuthType auth_type, QWidget *parent) : QDialog(parent), ui(new Ui::AuthDialog)
{
    ui->setupUi(this);
    setWindowTitle("Авторизация");

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    ui->line_login->setText(settings->value("auth/login").toString());

    switch (auth_type)
    {
    case AuthType::Standart:
        ui->error_label->setText("");
        break;
    case AuthType::EmptyField:
        ui->error_label->setText("Заполните логин и пароль");
        break;
    case AuthType::NotCorrectPassword:
        ui->error_label->setText("Некорректно введенные данные");
        break;
    case AuthType::NotFoundUser:
        ui->error_label->setText("Пользователь с таким логином не найден");
        break;
    }
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

QString AuthDialog::login() const
{
    return ui->line_login->text();
}

QString AuthDialog::password() const
{
    return ui->line_password->text();
}

QString AuthDialog::token()
{
    return "";
}

void AuthDialog::on_buttonBox_accepted()
{
    if (!login().isEmpty())
    {
        settings->setValue("auth/login", login());
    }
}
