#ifndef PLAN_DOCUMENT_H
#define PLAN_DOCUMENT_H

#include <QSettings>
#include <QSqlQueryModel>
#include <QWidget>

#include "popup.h"

namespace Ui
{
class PlanDocument;
}

class PlanDocuments : public QWidget
{
    Q_OBJECT

public:
    explicit PlanDocuments(QWidget *parent = nullptr);
    ~PlanDocuments();

    void resizeFilter();

    void setFilter();

public slots:
    void update_data();

signals:

    void planDocumentActivated(int id, bool read_only = false);

    void documentCreating();

    void updateDB();

private slots:
    void on_filter_plan_doc_cellChanged(int row, int column);

    void on_button_open_clicked();

    void on_plan_documents_doubleClicked(const QModelIndex &index);

    void on_button_add_clicked();

    void on_button_edit_clicked();

    void on_button_del_clicked();

    void on_button_exp_clicked();

    void on_button_imp_clicked();

    void on_button_dub_clicked();

    void on_button_rep_with_kkrb_clicked();

    void on_button_rep_with_kosgu_clicked();

    void on_button_send_clicked();

    void on_button_sln_clicked();

    void on_button_merge_clicked();

private:
    Ui::PlanDocument *ui;
    QSqlQueryModel *mDocPlanModel;
    QSettings *settings;
    PopUp *popUp;

    QString mCurrentKgrbs;
    int mCurrentYear;

    struct ColumnData
    {
        QString name;
        QString type;
        int width;
    };
    QList<ColumnData> column_data;
};

#endif  // PLAN_DOCUMENT_H
