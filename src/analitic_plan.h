#ifndef ANALITIC_PLAN_H
#define ANALITIC_PLAN_H

#include <QMenu>
#include <QSqlTableModel>
#include <QWidget>

namespace Ui
{
class AnaliticPlan;
}

class AnaliticPlan : public QWidget
{
    Q_OBJECT

    struct ColumnData
    {
        QString name;
        QString type;
        int width;
    };

public:
    enum Type
    {
        PlanData,
        PlanPivotData,
        PlanPivotDataDocument,
        FinanceDataDocumentYear,
        FinanceDataDocumentMonth,
        RemnantsIncrementallyYear,
        RemnantsIncrementallyMonth,
        RemnantsIncrementallyPeriod
    };

    explicit AnaliticPlan(Type type, QWidget *parent = nullptr);
    ~AnaliticPlan();

    void resizeColSum();

    void resizeTableCol(const int &index);

    void updateFilter();

    void add_exp_part_in_menu();

    void on_action_add_exp_part(const QString &exp_part);

    void closeEvent(QCloseEvent *event) override;

public slots:
    void customContextMenuRequested(const QPoint &pos);

private slots:
    void on_action_add_triggered();

    void on_accept_period_clicked();

private:
    Ui::AnaliticPlan *ui;

    QTimer *filterDebounceTimer;
    QSqlTableModel *mModel;
    QSqlQueryModel *mQueryModel;
    Type mType;
    QList<ColumnData> column_data;
    QMenu *rightClickMenu;
};

#endif  // ANALITIC_PLAN_H
