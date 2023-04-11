#ifndef EDITOR_FACT_H
#define EDITOR_FACT_H

#include <QMenu>
#include <QSqlTableModel>
#include <QWidget>

#include "model_kkrb.h"
#include "popup.h"
#include "sql_queries.h"
#include "ui_editor_fact.h"

namespace Ui
{
class EditorFact;
}

class EditorFact : public QWidget
{
    Q_OBJECT

public:
    explicit EditorFact(int id, bool read_only = false, QWidget *parent = nullptr);
    ~EditorFact();

    int getDocNum();

    void setCurrentMonth();

    void setTypeDoc();

    void insertToExpensesEditor();

    void setDropDownTable(QComboBox *rComboBox, const QString &rQuery);

    void resizeColSum();

    bool checkCode();

    void closeEvent(QCloseEvent *event) override;

    void setDirectionExpenditure();

    void resizeRowHeight();

    void setExpensesPart();

    QString getExpensesPart() const;

    bool checkIdDoc() const;

    bool saveData();

    bool checkDuplicate();

    void pasteFromClipboard(QTableView *view, Qt::ItemDataRole role = Qt::DisplayRole);

signals:

    void updateData();

public slots:

    void updateSum();

    void resizeColumnSumWidth(int ind);

private slots:

    void on_cb_kubp_currentIndexChanged(int index);

    void on_cb_krp_currentIndexChanged(int index);

    void on_cb_kcs_currentIndexChanged(int index);

    void on_cb_kvr_currentIndexChanged(int index);

    void on_cb_kosgu_currentIndexChanged(int index);

    void on_cb_notes_currentIndexChanged(int index);

    void on_add_kkrb_clicked();

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>());

    void on_resize_column_button_clicked();

    void on_del_str_fact_table_clicked();

    void on_cb_exp_part_currentIndexChanged(int index);

    void on_save_button_clicked();

    void on_filter_row_cellChanged(int row, int column);

    void on_filter_clear_clicked();

    void customContextMenuRequested(const QPoint &pos);

    void on_action_add_triggered();

    void on_action_NULL_triggered();

    void on_minus_fact_button_clicked();

private:
    Ui::EditorFact *ui;
    QSqlTableModel *mFactModel;
    QSqlQueryModel *mBalanceModel;
    QTimer *totalsDebounceTimer;
    PopUp *popUp;
    QMenu *rightClickMenu;

    bool dirty;
    int mIdDoc;
    bool mRefund;
    int mCurrentMonth;
    QString mFilter;
    QString mLastError;

    const int mWidthDropDown = 800;  // minimal width for dropping table
};

#endif  // EDITOR_FACT_H
