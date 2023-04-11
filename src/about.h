#ifndef ABOUT_H
#define ABOUT_H

#include <QAbstractButton>
#include <QDialog>
#include <QProcess>

namespace Ui
{
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

private slots:
    void on_pushButton_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::About *ui;
    QProcess *process;
};

#endif  // ABOUT_H
