#ifndef COLORPANEL_H
#define COLORPANEL_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QScreen>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QColor>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QStyle>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSettings>
#include "colorpicker.h"
#include "stylesheethelper.h"
#include "colorpickerhistory.h"
#include "colorbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ColorPanel; }
QT_END_NAMESPACE

class ColorPanel : public QMainWindow
{
    Q_OBJECT

public:
    ColorPanel(QWidget *parent = nullptr);
    ~ColorPanel();

private slots:
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void ColorPicked(QColor color);
    void on_closeWidgetButton_clicked();
    void on_toggleWindowMode_clicked();
    void on_pickColorButton_clicked();
    void ColorPickedFromHistory(QColor color);
    void ColorHoveredFromHistory(QColor color);
    void ColorUnhoveredFromHistory();
    void ColorPickedFromToolbar(int index);
    void PickerCancelled();

    void on_stayOnTopButton_toggled(bool checked);

    void on_clearHistoryButton_clicked();

    void on_colorFormatSelector_currentIndexChanged(int index);

private:
    Ui::ColorPanel *ui;
    
    QSettings * settings;

    QSystemTrayIcon *sysTray;
    QAction *openAction;
    QAction *quitAction;
    QMenu *trayMenu;

    QList<ColorPicker*> *pickerWindows;
    QList<QImage*> *desktopsImages;
    ColorPickerHistory * colorPickerHistory;

    StylesheetHelper* toolbarStyle;
    StylesheetHelper* toolbarButtonsDefaultStyle;
    StylesheetHelper* contentStyle;

    bool toolbarMode = true;
    bool restoreAfterPick = false;

    int historyColumns;

    void CreateTrayIcon();
    void CreateTrayActions();
    void ShowPickerWindows();
    void UpdateWindowMode();
    void FillHistory();
    void FillToolbarHistory();
    void ShowNotification(QString title, QString message);
    QString GetColorString(QColor color);
};
#endif // COLORPANEL_H
