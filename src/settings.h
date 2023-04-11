#ifndef SETTINGS_H
#define SETTINGS_H

#include <QComboBox>
#include <QSettings>
#include <QWidget>

#include "popup.h"

namespace Ui
{
class settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

    void setDropDownTable(QComboBox *rComboBox, const QString &rQuery);

signals:
    void update_data();

private slots:
    void on_cb_kgrbs_currentIndexChanged(int index);

    void on_year_editingFinished();

    void on_browse_update_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::settings *ui;
    QSettings *settings;
    PopUp *popUp;

    const int mWidthDropDown = 800;
};

#endif  // SETTINGS_H
