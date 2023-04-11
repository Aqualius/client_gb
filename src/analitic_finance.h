#ifndef ANALITIC_FINANCE_H
#define ANALITIC_FINANCE_H

#include <QMenu>
#include <QSqlTableModel>
#include <QWidget>

namespace Ui
{
class AnaliticFinance;
}

struct ColumnData
{
    QString name;
    QString type;
    int width;
};

class AnaliticFinance : public QWidget
{
    Q_OBJECT

public:
    explicit AnaliticFinance(QWidget *parent = nullptr);
    ~AnaliticFinance();

    void resizeColSum();

    void resizeTableCol(const int &ind);

    void on_action_add_exp_part(const QString &exp_part);

public slots:

    void updateFilter();

    void add_exp_part_in_menu();

    void customContextMenuRequested(const QPoint &pos);

private slots:
    void on_action_add_triggered();

private:
    Ui::AnaliticFinance *ui;
    QTimer *filterDebounceTimer;

    QSqlTableModel *mModel;
    QSqlQueryModel *mIncModel;
    QList<ColumnData> mColumnNames;
    QMenu *rightClickMenu;
};

#endif  // ANALITIC_FINANCE_H
