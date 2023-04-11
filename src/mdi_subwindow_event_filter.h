#ifndef MDISUBWINDOWEVENTFILTER_H
#define MDISUBWINDOWEVENTFILTER_H

#include <QMdiSubWindow>

class MdiSubWindowEventFilter : public QMdiSubWindow
{
    Q_OBJECT

public:
    MdiSubWindowEventFilter(QWidget *parent = nullptr);

signals:
    void closeSubWindow(QMdiSubWindow *);

protected:
    bool eventFilter(QObject *target, QEvent *event) override;
};

#endif  // MDISUBWINDOWEVENTFILTER_H
