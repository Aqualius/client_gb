#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QProcess>
#include <QTimer>

#include "mdi_subwindow_event_filter.h"
#include "popup.h"
#include "request_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void start_update(const QString &path);

    void onSuccessfulInit();

private slots:
    void on_editor_plan_triggered();

    void on_editor_fact_triggered();

    void planDocumentOpen(int id, bool read_only = false);

    void factDocumentOpen(int id, bool read_only = false);

    void on_analitic_plan_triggered();

    void on_analitic_plan_month_triggered();

    void on_plan_doc_month_triggered();

    void on_fact_month_triggered();

    void on_fact_year_triggered();

    void on_inc_year_triggered();

    void on_inc_month_triggered();

    void on_inc_period_triggered();

    ///// ABOUT /////
    void on_about_triggered();

    void on_settings_triggered();

    void whenTabClose(QMdiSubWindow *win);

    void on_local_triggered();

    void on_not_local_triggered();

    void check_token();

    void run_auth(AuthType auth_type);

    void on_action_triggered();

    void on_check_update_triggered();

    void check_update_programm(bool show_message = false);

    void on_editor_report_triggered();

    void closeEvent(QCloseEvent *event);

    void on_from_file_triggered();

signals:

    void update_data();

private:
    Ui::MainWindow *ui;
    RequestManager *req_manager;
    PopUp *popUp;
    MdiSubWindowEventFilter *p_mdiSubWindowEventFilter;
    QTimer *totalsUpdateTimer;
};
#endif  // MAINWINDOW_H
