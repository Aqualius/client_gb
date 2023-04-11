#include "mainwindow.h"

#include <LimeReport>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMessageBox>

#include "./ui_mainwindow.h"
#include "about.h"
#include "analitic_finance.h"
#include "analitic_plan.h"
#include "auth_dialog.h"
#include "editor_fact.h"
#include "editor_plan.h"
#include "fact_document.h"
#include "plan_document.h"
#include "request_manager.h"
#include "settings.h"
#include "updater.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/client-gb-logo.png"));
    setWindowTitle("Клиент ГБ");

    QSettings settings("settings.ini", QSettings::IniFormat);
    if (settings.value("settings/year").toString().size() != 4)
    {
        settings.setValue("settings/year", QDate::currentDate().toString("yyyy"));
    }

    popUp = new PopUp(this);

    totalsUpdateTimer = new QTimer(this);
    totalsUpdateTimer->setInterval(1000 * 60 * 60);
    totalsUpdateTimer->setSingleShot(true);
    connect(totalsUpdateTimer, &QTimer::timeout, this, [=]() { MainWindow::check_update_programm(); });

    check_update_programm();

    ////// REQUEST //////
    req_manager = new RequestManager(this);
    connect(req_manager, &RequestManager::authToken, this, &MainWindow::run_auth);
    connect(req_manager, &RequestManager::updateLocal, this, &MainWindow::start_update);

    ////// CONNECT EVENT FILTER //////
    p_mdiSubWindowEventFilter = new MdiSubWindowEventFilter(this);
    connect(p_mdiSubWindowEventFilter, &MdiSubWindowEventFilter::closeSubWindow, this, &MainWindow::whenTabClose);
    p_mdiSubWindowEventFilter->close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_editor_plan_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("name") == "EditorPlan")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    PlanDocuments *doc = new PlanDocuments(ui->tab_panel);

    doc->setProperty("name", "EditorPlan");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(doc);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    connect(doc, &PlanDocuments::planDocumentActivated, this, &MainWindow::planDocumentOpen);
    connect(doc, &PlanDocuments::updateDB, this, &MainWindow::on_not_local_triggered);
    connect(this, &MainWindow::update_data, doc, &PlanDocuments::update_data);
    doc->showMaximized();
}

void MainWindow::on_editor_fact_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("name") == "EditorFact")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    FactDocuments *doc = new FactDocuments(ui->tab_panel);

    doc->setProperty("name", "EditorFact");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(doc);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    connect(doc, &FactDocuments::factDocumentActivated, this, &MainWindow::factDocumentOpen);
    connect(doc, &FactDocuments::updateDB, this, &MainWindow::on_not_local_triggered);
    connect(this, &MainWindow::update_data, doc, &FactDocuments::update_data);
    doc->showMaximized();
}

void MainWindow::planDocumentOpen(int id, bool read_only)
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == id)
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    EditorPlan *ep = new EditorPlan(id, read_only, this);
    ep->setProperty("id", id);
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::factDocumentOpen(int id, bool read_only)
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == id)
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    EditorFact *ep = new EditorFact(id, read_only, this);
    ep->setProperty("id", id);
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    connect(ep, &EditorFact::updateData, this, [=]() { emit MainWindow::update_data(); });
    ep->showMaximized();
}

void MainWindow::on_analitic_plan_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "analitic_plan")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::PlanData, this);
    ep->setProperty("id", "analitic_plan");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_analitic_plan_month_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "analitic_plan_pivot_data")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::PlanPivotData, this);
    ep->setProperty("id", "analitic_plan_pivot_data");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_plan_doc_month_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "analitic_plan_pivot_data_document")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::PlanPivotDataDocument, this);
    ep->setProperty("id", "analitic_plan_pivot_data_document");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_fact_month_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "analitic_finance_month")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::FinanceDataDocumentMonth, this);
    ep->setProperty("id", "analitic_finance_month");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_fact_year_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "analitic_finance_year")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::FinanceDataDocumentYear, this);
    ep->setProperty("id", "analitic_finance_year");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_inc_year_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "remnants_incrementally_year")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::RemnantsIncrementallyYear, this);
    ep->setProperty("id", "remnants_incrementally_year");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_inc_month_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "remnants_incrementally_month")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticFinance *ep = new AnaliticFinance(this);
    ep->setProperty("id", "remnants_incrementally_month");
    ui->tab_panel->addSubWindow(ep);
    ep->showMaximized();
}

void MainWindow::on_inc_period_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "remnants_incrementally_period")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    AnaliticPlan *ep = new AnaliticPlan(AnaliticPlan::RemnantsIncrementallyPeriod, this);
    ep->setProperty("id", "remnants_incrementally_period");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(ep);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    ep->showMaximized();
}

void MainWindow::on_about_triggered()
{
    About *about_dialog = new About(this);
    about_dialog->show();
    //    QMessageBox::information(this, "О программе",
    //                             QString("Дата компиляции: %1 в %2 \n"
    //                                     "Версия программы: %3")
    //                                 .arg(__DATE__, __TIME__, "1.9.5"));
}

void MainWindow::on_settings_triggered()
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        if (tab->widget()->property("id") == "settings")
        {
            ui->tab_panel->setActiveSubWindow(tab);
            return;
        }
    }
    Settings *s = new Settings();
    s->setProperty("id", "settings");
    QMdiSubWindow *subWin = ui->tab_panel->addSubWindow(s);
    subWin->installEventFilter(p_mdiSubWindowEventFilter);
    connect(s, &Settings::update_data, this, [=]() { emit MainWindow::update_data(); });
    s->showMaximized();
}

void MainWindow::whenTabClose(QMdiSubWindow *win)
{
    if (win != ui->tab_panel->activeSubWindow())
    {
        return;
    }
    QMdiSubWindow *new_win = nullptr;
    const auto lists = ui->tab_panel->subWindowList();
    for (const auto &win : lists)
    {
        if (win == ui->tab_panel->activeSubWindow())
        {
            break;
        }
        new_win = win;
    }
    if (new_win == nullptr)
    {
        return;
    }
    ui->tab_panel->setActiveSubWindow(new_win);
}

void MainWindow::start_update(const QString &path)
{
    QFileInfo check_file(path);
    if (!check_file.exists() || !check_file.isFile())
    {
        QMessageBox::critical(this, "Ошибка",
                              "Файл обновления не обнаружен. Попробуйте запустить заново удаленное обновление!");
        return;
    }
    QWidget::setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    Updater updater(path);
    bool checker = updater.update_base();
    QApplication::restoreOverrideCursor();
    QWidget::setEnabled(true);
    if (checker)
    {
        popUp->setPopupType(Access);
        popUp->setPopupText("Данные успешно обновлены!");
        popUp->show();
        emit update_data();
    }
    else
    {
        popUp->setPopupType(Error);
        popUp->setPopupText("Обновление не удалось!");
        popUp->show();
    }
}

void MainWindow::on_local_triggered()
{
    check_token();
}

void MainWindow::on_not_local_triggered()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString program_path = "./gbupd/GBUpdater.exe";
    QStringList arguments;
    arguments << "-year" << settings.value("settings/year").toString() << "-extract-to"
              << "../update"
              << "-db-format"
              << "cgb_2022"
              << "-run"
              << "-no-client-gb";
    QProcess *process = new QProcess(this);
    process->setWorkingDirectory("./gbupd");
    process->start(program_path, arguments);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit)
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Процесс обновления завершен некорректно. Детали:\n" + process->errorString());
                    popUp->show();
                    return;
                }
                else if (exitCode != 0)
                {
                    popUp->setPopupType(Error);
                    popUp->setPopupText("Процесс обновления завершен некорректно. Детали:\n" + process->errorString());
                    popUp->show();
                    return;
                }
                start_update("./update/cgb_update.zip");
            });
}

void MainWindow::check_token()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    QString token = settings.value("update/token").toString();
    req_manager->validate_token(token);
}

void MainWindow::run_auth(AuthType auth_type)
{
    AuthDialog auth(auth_type);
    int res = auth.exec();
    if (res == QDialog::Accepted)
    {
        req_manager->token(auth.login(), auth.password());
    }
}

void MainWindow::on_action_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath() + "/README.pdf"));
}

void MainWindow::on_check_update_triggered()
{
    check_update_programm(true);
}

void MainWindow::check_update_programm(bool show_message)
{
    auto process = Updater::createProgramUpdateChecker(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                auto p = static_cast<QProcess *>(sender());
                QString output(p->readAllStandardOutput());
                qDebug() << "Updater output:" << output;

                if (output.contains("Update is required"))
                {
                    int confirm =
                        QMessageBox::warning(this, "Обновление",
                                             "Доступно обновление программы.<br>"
                                             "Во время обновления не закрывайте открывшееся окно команд.<br><br>"
                                             "Начать обновление?",
                                             QMessageBox::Yes | QMessageBox::No);

                    if (confirm == QMessageBox::Yes)
                    {
                        Updater::runProgramUpdater();
                        QCoreApplication::quit();
                    }
                }
                else if (show_message)
                {
                    popUp->setPopupType(Access);
                    popUp->setPopupText("Вы используете актуальную версию програмы");
                    popUp->show();
                }
            });
    process->start();
    totalsUpdateTimer->start();
}

void MainWindow::on_editor_report_triggered()
{
    QString path =
        QFileDialog::getOpenFileName(0, "Открыть файл", "D:/Project/QT/client_gb/reports", "File Report(*.lrxml)");
    if (!path.isEmpty())
    {
        auto report = new LimeReport::ReportEngine(this);
        report->loadFromFile(path);
        report->designReport();
        popUp->setPopupType(Inforamation);
        popUp->setPopupText("Если отчеты уже есть в программе, то нужно пересобирать программу!");
        popUp->show();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        tab->close();
    }
    if (ui->tab_panel->subWindowList().size() != 0)
    {
        event->ignore();
    }
}

void MainWindow::on_from_file_triggered()
{
    QString path = QFileDialog::getOpenFileName(0, "Открыть файл", QDir::currentPath(), "ZIP(*.zip)");
    if (path == "")
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Файл не был выбран!");
        popUp->show();
        return;
    }
    const auto sub_win_list = ui->tab_panel->subWindowList();
    for (auto tab : sub_win_list)
    {
        tab->close();
    }
    if (ui->tab_panel->subWindowList().size() > 0)
    {
        popUp->setPopupType(Warning);
        popUp->setPopupText("Закройте все вкладки, иначе обновление не начнется!");
        popUp->show();
        return;
    }
    start_update(path);
}
