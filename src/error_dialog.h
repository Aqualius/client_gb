#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui
{
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();

    bool check_plan(int id);

private:
    Ui::ErrorDialog *ui;
    QSqlQueryModel *mPlanErrorModel;
    QSqlQueryModel *mRemnErrorModel;
};

#endif  // ERROR_DIALOG_H
