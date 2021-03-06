#include "colorpickertitlebar.h"

ColorPickerTitlebar::ColorPickerTitlebar(QWidget *parent) : QWidget(parent)
{

}

void ColorPickerTitlebar::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ColorPickerTitlebar::mousePressEvent(QMouseEvent *event)
{
    pressPos= event->pos();
    draggingWindow = true;
}

void ColorPickerTitlebar::mouseReleaseEvent(QMouseEvent *)
{
    draggingWindow = false;
}

void ColorPickerTitlebar::mouseMoveEvent(QMouseEvent *event)
{
    if(draggingWindow)
    {
        QList<QScreen*> screens = QGuiApplication::screens();
        int sIndex = PointerToScreen(event->globalPos());

        QPoint diff= event->pos() - pressPos;
        QPoint windowPosition = window()->pos() + diff;
        QRect availableGeometry = screens.at(sIndex)->availableGeometry();

        int wx = windowPosition.x(); // Window x position
        int wy = windowPosition.y(); // Window y position
        int ww = window()->width();
        int wh = window()->height();

        int minX = availableGeometry.x();
        int minY = availableGeometry.y();
        int maxX = minX + availableGeometry.width();
        int maxY = minY + availableGeometry.height();

        wx = wx < minX ? minX : wx;
        wx = wx + ww > maxX ? maxX - ww : wx;
        wy = wy < minY ? minY : wy;
        wy = wy + wh > maxY ? maxY - wh : wy;

        windowPosition.setX(wx);
        windowPosition.setY(wy);
        window()->move(windowPosition);
    }
}

int ColorPickerTitlebar::PointerToScreen(QPoint pointer)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    for (int sIndex = 0; sIndex < screens.count(); sIndex++) {
        QScreen *screen = screens.at(sIndex);
        QRect space = screen->geometry();
        if(
            pointer.x() >= space.x() &&
            pointer.x() <= space.x() + space.width() &&
            pointer.y() >= space.y() &&
            pointer.y() <= space.y() + space.height()
        )
        {
            return sIndex;
        }
    }
}
