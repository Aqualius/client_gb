#ifndef PLAN_DOCUMENT_EDITOR_H
#define PLAN_DOCUMENT_EDITOR_H

#include <QComboBox>
#include <QDialog>
#include <QSettings>

namespace Ui
{
class PlanDocumentEditor;
}

class PlanDocumentEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PlanDocumentEditor(QWidget *parent = nullptr, int id = -1);
    ~PlanDocumentEditor();

    QString docNumber() const;

    QString docDate() const;

    QString basisUid() const;

    QString userComment() const;

    void accept() override;

    void setDropDownTable(QComboBox *rComboBox, const QString &rQuery);

    bool check_number();
private slots:
    void on_basis_uid_currentIndexChanged(int index);

    void on_doc_number_currentIndexChanged(const QString &arg1);

private:
    Ui::PlanDocumentEditor *ui;
    QSettings *settings;
    QString mCurrentKgrbs;
    QString mCurrentUid;
    QString mCurrentDocNumber;
    int mCurrentYear;
    int mIdDoc;
    const int mWidthDropDown = 450;
};

#endif  // PLAN_DOCUMENT_EDITOR_H
