#include "editor_signature.h"

#include <QSettings>

#include "ui_editor_signature.h"

EditorSignature::EditorSignature(ReportType type, QWidget *parent)
    : mReportType(type), QDialog(parent), ui(new Ui::EditorSignature)
{
    ui->setupUi(this);

    setWindowTitle("Правка данных подписантов");

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    if (mReportType == PLAN)
    {
        ui->signatory_fio_1->setText(settings->value("report_plan/fio_1").toString());
        ui->signatory_fio_2->setText(settings->value("report_plan/fio_2").toString());
        ui->signatory_post_1->setText(settings->value("report_plan/post_1").toString());
        ui->signatory_post_2->setText(settings->value("report_plan/post_2").toString());
        ui->signatory_fio_3->setVisible(false);
        ui->signatory_post_3->setVisible(false);
        ui->signatory_tel_1->setVisible(false);
        ui->signatory_tel_2->setVisible(false);
        ui->signatory_tel_3->setVisible(false);
        ui->label_fio_3->setVisible(false);
        ui->label_post_3->setVisible(false);
        ui->label_tel_1->setVisible(false);
        ui->label_tel_2->setVisible(false);
        ui->label_tel_3->setVisible(false);
    }
    else if (mReportType == FACT)
    {
        ui->signatory_fio_1->setText(settings->value("report_fact/fio_1").toString());
        ui->signatory_fio_2->setText(settings->value("report_fact/fio_2").toString());
        ui->signatory_fio_3->setText(settings->value("report_fact/fio_3").toString());
        ui->signatory_tel_1->setText(settings->value("report_fact/tel_1").toString());
        ui->signatory_tel_2->setText(settings->value("report_fact/tel_2").toString());
        ui->signatory_tel_3->setText(settings->value("report_fact/tel_3").toString());
        ui->signatory_post_1->setText(settings->value("report_fact/post_1").toString());
        ui->signatory_post_2->setText(settings->value("report_fact/post_2").toString());
        ui->signatory_post_3->setText(settings->value("report_fact/post_3").toString());
    }
}

EditorSignature::~EditorSignature()
{
    delete ui;
}

void EditorSignature::on_buttonBox_accepted()
{
    if (mReportType == PLAN)
    {
        settings->setValue("report_plan/post_1", ui->signatory_post_1->toPlainText());
        settings->setValue("report_plan/post_2", ui->signatory_post_2->toPlainText());
        settings->setValue("report_plan/fio_1", ui->signatory_fio_1->text());
        settings->setValue("report_plan/fio_2", ui->signatory_fio_2->text());
    }
    else if (mReportType == FACT)
    {
        settings->setValue("report_fact/post_1", ui->signatory_post_1->toPlainText());
        settings->setValue("report_fact/post_2", ui->signatory_post_2->toPlainText());
        settings->setValue("report_fact/post_3", ui->signatory_post_3->toPlainText());
        settings->setValue("report_fact/fio_1", ui->signatory_fio_1->text());
        settings->setValue("report_fact/fio_2", ui->signatory_fio_2->text());
        settings->setValue("report_fact/fio_3", ui->signatory_fio_3->text());
        settings->setValue("report_fact/tel_1", ui->signatory_tel_1->text());
        settings->setValue("report_fact/tel_2", ui->signatory_tel_2->text());
        settings->setValue("report_fact/tel_3", ui->signatory_tel_3->text());
    }
    this->close();
}
