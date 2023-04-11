#ifndef FACT_DOCUMENT_EDITOR_H
#define FACT_DOCUMENT_EDITOR_H

#include <QDialog>

namespace Ui
{
class FactDocumentEditor;
}

class FactDocumentEditor : public QDialog
{
    Q_OBJECT

public:
    explicit FactDocumentEditor(QWidget *parent = nullptr, int id = -1);
    ~FactDocumentEditor();

    QString docNumber();

    QString docDate();

    QString userComment();

    QString paymentType();

    void accept() override;

    bool check_number();

private:
    Ui::FactDocumentEditor *ui;
    int mDocId;
};

#endif  // FACT_DOCUMENT_EDITOR_H
