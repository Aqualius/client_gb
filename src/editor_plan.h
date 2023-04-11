#ifndef EDITOR_PLAN_H
#define EDITOR_PLAN_H

#include <QMenu>
#include <QSettings>
#include <QSqlTableModel>
#include <QWidget>

#include "model_kkrb.h"
#include "popup.h"
#include "sql_queries.h"
#include "ui_editor_plan.h"

namespace Ui
{
class EditorPlan;
}

class EditorPlan : public QWidget
{
    Q_OBJECT

public:
    explicit EditorPlan(int id, bool read_only = false, QWidget *parent = nullptr);
    ~EditorPlan();

    int getDocNum() const;

    void insertToExpensesEditor();

    void setDropDownTable(QComboBox *rComboBox, const QString &rQuery);

    bool checkCode() const;

    bool checkKosgu() const;

    void closeEvent(QCloseEvent *event) override;

    void resizeRowHeight();

    void setExpensesPart();

    QString getExpensesPart() const;

    bool checkIdDoc() const;

    QSqlError saveData();

    bool checkDuplicate();

    void pasteFromClipboard(QTableView *view, Qt::ItemDataRole role = Qt::DisplayRole);

public slots:

    void updateSum();

    void resizeColWidth();

    void resizeColumnSumWidth(int ind);

private slots:

    void on_cb_kubp_currentIndexChanged(int index);

    void on_cb_krp_currentIndexChanged(int index);

    void on_cb_kcs_currentIndexChanged(int index);

    void on_cb_kvr_currentIndexChanged(int index);

    void on_cb_kosgu_currentIndexChanged(int index);

    void on_add_kkrb_clicked();

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>());

    void on_delete_row_button_clicked();

    void on_check_button_clicked();

    void on_save_button_clicked();

    void on_minus_plan_button_clicked();

    void on_filter_clear_clicked();

    void on_filter_row_cellChanged(int row, int column);

    void customContextMenuRequested(const QPoint &pos);

    void on_action_NULL_triggered();

    void on_action_add_triggered();

    void on_minus_ost_button_clicked();

private:
    Ui::editor_plan *ui;
    QSqlTableModel *mPlanModel;
    QSqlQueryModel *mYearSumModel;
    QTimer *totalsDebounceTimer;
    QSettings *settings;
    PopUp *popUp;
    QMenu *rightClickMenu;

    bool dirty;
    bool validation_not_runned;
    int mIdDoc;
    int mCurrentYear;
    QString mFilter;
    const int mWidthDropDown = 800;  // minimal width for dropping table
};

#endif  // EDITOR_PLAN_H
