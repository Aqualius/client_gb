#include "mdi_subwindow_event_filter.h"

#include <QEvent>

MdiSubWindowEventFilter::MdiSubWindowEventFilter(QWidget *parent) : QMdiSubWindow(parent)
{
}

bool MdiSubWindowEventFilter::eventFilter(QObject *target, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Close: {
        QMdiSubWindow *subWindow = dynamic_cast<QMdiSubWindow *>(target);
        Q_ASSERT(subWindow != NULL);

        emit closeSubWindow(subWindow);
        break;
    }
    default:
        qt_noop();
    }
    return QObject::eventFilter(target, event);
}
