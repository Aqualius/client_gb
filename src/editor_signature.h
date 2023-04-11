#ifndef EDITOR_SIGNATURE_H
#define EDITOR_SIGNATURE_H

#include <QDialog>
#include <QSettings>

enum ReportType
{
    PLAN,
    FACT
};

namespace Ui
{
class EditorSignature;
}

class EditorSignature : public QDialog
{
    Q_OBJECT

public:
    explicit EditorSignature(ReportType type, QWidget *parent = nullptr);
    ~EditorSignature();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::EditorSignature *ui;

    QSettings *settings;

    ReportType mReportType;
};

#endif  // EDITOR_SIGNATURE_H
