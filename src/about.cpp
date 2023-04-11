#include "about.h"

#include "ui_about.h"
#include "updater.h"

About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About)
{
    ui->setupUi(this);

    QString ver = "Версия 1.9.10";
    ui->label->setText(ver);
    ui->label_2->setText(ver);
    ui->label_5->setText(ver);

    ui->label_2->setVisible(false);
    ui->label_4->setVisible(false);
    ui->label_5->setVisible(false);
    ui->pushButton->setVisible(false);

    process = Updater::createProgramUpdateChecker(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                auto p = static_cast<QProcess *>(sender());
                QString output(p->readAllStandardOutput());

                ui->label->setVisible(false);
                ui->label_3->setVisible(false);
                if (output.contains("Update is required"))
                {
                    ui->label_5->setVisible(true);
                    ui->pushButton->setVisible(true);
                }
                else
                {
                    ui->label_2->setVisible(true);
                    ui->label_4->setVisible(true);
                }
            });
    process->start();

    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::rejected);
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    Updater::runProgramUpdater();
    QCoreApplication::quit();
}

void About::on_buttonBox_clicked(QAbstractButton *button)
{
    accept();
}
