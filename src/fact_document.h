#ifndef FACT_DOCUMENT_H
#define FACT_DOCUMENT_H

#include <QSettings>
#include <QSqlQueryModel>
#include <QWidget>

#include "popup.h"

namespace Ui
{
class FactDocument;
}

class FactDocuments : public QWidget
{
    Q_OBJECT

public:
    explicit FactDocuments(QWidget *parent = nullptr);
    ~FactDocuments();

    void resizeFilter();

    void setFilter();

public slots:
    void update_data();

signals:

    void factDocumentActivated(int id, bool read_only = false);

    void documentCreating();

    void updateDB();

private slots:
    void on_filter_fact_doc_cellChanged(int row, int column);

    void on_button_open_clicked();

    void on_fact_documents_doubleClicked(const QModelIndex &index);

    void on_button_add_clicked();

    void on_button_edit_clicked();

    void on_button_del_clicked();

    void on_button_exp_clicked();

    void on_button_imp_clicked();

    void on_button_dub_clicked();

    void on_button_rep_clicked();

    void on_button_rep_alter_clicked();

    void on_button_send_clicked();

    void on_button_merge_clicked();

private:
    Ui::FactDocument *ui;
    QSqlQueryModel *mDocFactModel;
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

#endif  // FACT_DOCUMENT_H
