#include "colorpanel.h"
#include "ui_colorpanel.h"

ColorPanel::ColorPanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ColorPanel)
{
    ui->setupUi(this);

    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    settings = new QSettings("Konkorporation", "ColorPicker");
    int colorFormatIndex = settings->value("picker/colorformat", 0).toInt();
    ui->colorFormatSelector->setCurrentIndex(colorFormatIndex);

    CreateTrayActions(); // Create tray menu actions
    CreateTrayIcon(); // Create the actual tray icon to be displayed
    connect(sysTray, &QSystemTrayIcon::activated, this, &ColorPanel::trayActivated); // This connects the events of the tray to trayActivated method

    int screenCount = QGuiApplication::screens().count(); // Count the number of screens
    pickerWindows = new QList<ColorPicker*>(); // Initialize the ColorPicker Widgets (Windows)
    for(int wIndex = 0; wIndex < screenCount; wIndex++)
    {
        ColorPicker *colorPicker = new ColorPicker();
        connect(colorPicker, &ColorPicker::ColorPicked, this, &ColorPanel::ColorPicked);
        connect(colorPicker, &ColorPicker::PickerCancelled, this, &ColorPanel::PickerCancelled);
        colorPicker->setMouseTracking(true);
        pickerWindows->push_back(colorPicker);
    }

    qDebug() << "Added " << screenCount << " screens, Total screens:" << pickerWindows->count();

    toolbarStyle = new StylesheetHelper();
    toolbarStyle->SetRule(new QString("background-color"), new QString("#2D3136"));
    toolbarStyle->SetRule(new QString("border-top-left-radius"), new QString("6px"));
    toolbarStyle->SetRule(new QString("border-top-right-radius"), new QString("6px"));
    toolbarStyle->SetRule(new QString("border-bottom-right-radius"), new QString("6px"));
    toolbarStyle->SetRule(new QString("border-bottom-left-radius"), new QString("6px"));

    toolbarButtonsDefaultStyle = new StylesheetHelper();
    toolbarButtonsDefaultStyle->SetRule(new QString("width"), new QString("30px"));
    toolbarButtonsDefaultStyle->SetRule(new QString("height"), new QString("30px"));
    toolbarButtonsDefaultStyle->SetRule(new QString("border"), new QString("1px solid transparent"));
    toolbarButtonsDefaultStyle->SetRule(new QString("background-image"), new QString("url(:/images/outline.png)"));
    toolbarButtonsDefaultStyle->SetRule(new QString("background-repeat"), new QString("no-repeat"));
    toolbarButtonsDefaultStyle->SetRule(new QString("background-color"), new QString("transparent"));

    contentStyle = new StylesheetHelper();
    contentStyle->SetRule(new QString("background-color"), new QString("#EEE"));
    contentStyle->SetRule(new QString("border"), new QString("1px solid #CCC"));
    contentStyle->SetRule(new QString("border-top-left-radius"), new QString("0px"));
    contentStyle->SetRule(new QString("border-top-right-radius"), new QString("0px"));
    contentStyle->SetRule(new QString("border-bottom-right-radius"), new QString("6px"));
    contentStyle->SetRule(new QString("border-bottom-left-radius"), new QString("6px"));

    ui->contentWidget->setStyleSheet(contentStyle->BuildStylesheet());

    UpdateWindowMode();
    //this->hide();

    colorPickerHistory = new ColorPickerHistory();
    colorPickerHistory->LoadHistory();

    historyColumns = 7;

    FillHistory();
    FillToolbarHistory();

    int toolbarButtonIndex = 0;
    connect(ui->toolbarHistoryColor1, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
    toolbarButtonIndex++;
    connect(ui->toolbarHistoryColor2, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
    toolbarButtonIndex++;
    connect(ui->toolbarHistoryColor3, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
    toolbarButtonIndex++;
    connect(ui->toolbarHistoryColor4, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
    toolbarButtonIndex++;
    connect(ui->toolbarHistoryColor5, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
    toolbarButtonIndex++;
    connect(ui->toolbarHistoryColor6, &QPushButton::clicked, this, [this, toolbarButtonIndex]() {ColorPickedFromToolbar(toolbarButtonIndex);});
}

ColorPanel::~ColorPanel()
{
    delete ui;
}

void ColorPanel::CreateTrayActions()
{
    quitAction = new QAction(tr("&Quit"), this);
    openAction = new QAction(tr("&Open"), this);

    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(openAction, &QAction::triggered, this, &QWidget::showNormal);
}

void ColorPanel::CreateTrayIcon()
{
    trayMenu = new QMenu();
    trayMenu->addAction(openAction);
    trayMenu->addAction(quitAction);

    QIcon icon = QIcon(":/images/tray-icon.png");

    sysTray = new QSystemTrayIcon(this);
    sysTray->setContextMenu(trayMenu);
    sysTray->setIcon(icon);
    sysTray->setToolTip("Color picker");
    sysTray->show();
}

void ColorPanel::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        this->show();
        break;
    case QSystemTrayIcon::MiddleClick:
        ShowPickerWindows();
        break;
    default:
        ;
    }
}

void ColorPanel::ShowPickerWindows()
{
    int screenCount = QGuiApplication::screens().count();
    if(screenCount > pickerWindows->count())
    {
        int screensToAdd = screenCount - pickerWindows->count();
        for(int wIndex = 0; wIndex < screensToAdd; wIndex++)
        {
            ColorPicker *colorPicker = new ColorPicker();
            connect(colorPicker, &ColorPicker::ColorPicked, this, &ColorPanel::ColorPicked);
            colorPicker->setMouseTracking(true);
            pickerWindows->push_back(colorPicker);
        }
        qDebug() << "Added " << screensToAdd << " screens, Total screens:" << pickerWindows->count();
    }

    for(int wIndex = 0; wIndex < screenCount; wIndex++)
    {
        QScreen *screen = QGuiApplication::screens().at(wIndex);
        QRect screenRect = screen->geometry();
        ColorPicker *picker = pickerWindows->at(wIndex);
        picker->move(screenRect.x(), screenRect.y());
        picker->resize(screenRect.width(), screenRect.height());
        picker->setFixedSize(screenRect.width(), screenRect.height());

        QPixmap desktopImage = screen->grabWindow(QDesktopWidget().winId(),
                                                  screenRect.x(),
                                                  screenRect.y(),
                                                  screenRect.width(),
                                                  screenRect.height());
        picker->backgroundPixmap = desktopImage;

        picker->show();
        picker->activateWindow();
    }
}

void ColorPanel::ColorPicked(QColor color)
{

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(GetColorString(color));

    for(int wIndex = 0; wIndex < pickerWindows->count(); wIndex++)
    {
        ColorPicker *picker = pickerWindows->at(wIndex);
        picker->hide();
    }
    if(restoreAfterPick)
    {
        show();
        restoreAfterPick = false;
    }
    colorPickerHistory->PushToHistory(color);
    FillHistory();
    FillToolbarHistory();

    ShowNotification("Color copied to clipboard", GetColorString(color));
}

void ColorPanel::PickerCancelled()
{
    for(int wIndex = 0; wIndex < pickerWindows->count(); wIndex++)
    {
        ColorPicker *picker = pickerWindows->at(wIndex);
        picker->hide();
    }
}

void ColorPanel::ColorPickedFromHistory(QColor color)
{
    //qDebug() << "Picked from history: " << GetColorString(color);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(GetColorString(color));
    ShowNotification("Color copied to clipboard", GetColorString(color));
}

void ColorPanel::ColorPickedFromToolbar(int index)
{
    if(colorPickerHistory->history->count() - 1 >= index)
    {
        //qDebug() << (colorPickerHistory->history->count()-1) - index;
        QColor color = colorPickerHistory->history->at((colorPickerHistory->history->count()-1) - index);
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(GetColorString(color));
        ShowNotification("Color copied to clipboard", GetColorString(color));
    }
}

void ColorPanel::ColorHoveredFromHistory(QColor color)
{
    //qDebug() << "Hovered from history: " << GetColorString(color);
    ui->colorLabel->setText(GetColorString(color));
}

void ColorPanel::ColorUnhoveredFromHistory()
{
    ui->colorLabel->setText("");
}

void ColorPanel::UpdateWindowMode()
{
    if(toolbarMode)
    {
        resize(450, 40);
        toolbarStyle->SetRule(new QString("border-top-left-radius"), new QString("6px"));
        toolbarStyle->SetRule(new QString("border-top-right-radius"), new QString("6px"));
        toolbarStyle->SetRule(new QString("border-bottom-right-radius"), new QString("6px"));
        toolbarStyle->SetRule(new QString("border-bottom-left-radius"), new QString("6px"));

        ui->toolbarMain->setStyleSheet(toolbarStyle->BuildStylesheet());
        ui->toolbarColorsContainer->show();
        ui->toolbarTitleContainer->hide();
    }
    else
    {
        resize(630, 400);
        toolbarStyle->SetRule(new QString("border-top-left-radius"), new QString("6px"));
        toolbarStyle->SetRule(new QString("border-top-right-radius"), new QString("6px"));
        toolbarStyle->SetRule(new QString("border-bottom-right-radius"), new QString("0px"));
        toolbarStyle->SetRule(new QString("border-bottom-left-radius"), new QString("0px"));

        ui->toolbarMain->setStyleSheet(toolbarStyle->BuildStylesheet());
        ui->toolbarColorsContainer->hide();
        ui->toolbarTitleContainer->show();
    }
}

void ColorPanel::on_closeWidgetButton_clicked()
{
    hide();
}

void ColorPanel::on_toggleWindowMode_clicked()
{
    toolbarMode = !toolbarMode;
    UpdateWindowMode();
}

void ColorPanel::on_pickColorButton_clicked()
{
    hide();
    restoreAfterPick = true;
    ShowPickerWindows();
}

void ColorPanel::FillHistory()
{
    QGridLayout * layout = new QGridLayout();

    int row;
    int column;
    int historyCount = colorPickerHistory->history->count();
    int historyCountNormalized = historyCount - 1;
    for (int i = historyCount - 1; i >= 0; i--) {
        StylesheetHelper * buttonStyle = new StylesheetHelper();
        buttonStyle->SetRule(new QString("border"), new QString("1px solid #CCC"));
        buttonStyle->SetRule(new QString("border-radius"), new QString("6px"));
        buttonStyle->SetRule(new QString("width"), new QString("32px"));
        buttonStyle->SetRule(new QString("height"), new QString("32px"));

        QColor currentColor = colorPickerHistory->history->at(i);
        buttonStyle->SetRule(new QString("background-color"), new QString(currentColor.name(QColor::HexRgb)));

        row = (int)floor((historyCountNormalized - i)/historyColumns);
        column = (historyCountNormalized - i) % historyColumns;
        ColorButton * colorButton = new ColorButton();
        colorButton->setObjectName(QString("cSelector_%1").arg(i));
        colorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        colorButton->resize(32,32);
        colorButton->setStyleSheet(buttonStyle->BuildStylesheet());
        colorButton->setCursor(Qt::PointingHandCursor);

        connect(colorButton, &ColorButton::clicked, this, [this, currentColor]() {
            ColorPickedFromHistory(currentColor);
        });

        connect(colorButton, &ColorButton::hovered , this, [this, currentColor]() {
            ColorHoveredFromHistory(currentColor);
        });

        connect(colorButton, &ColorButton::unhovered , this, [this]() {
            ColorUnhoveredFromHistory();
        });

        layout->addWidget(colorButton, row, column, Qt::AlignTop);
    }

    QSpacerItem * spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, column+1, 1, -1, Qt::AlignTop);

    delete ui->scrollAreaWidgetContentsHistory->layout();
    qDeleteAll(ui->scrollAreaWidgetContentsHistory->children());

    ui->scrollAreaWidgetContentsHistory->setLayout(layout);
    //ui->scrollAreaWidgetContentsHistory->dumpObjectTree();
}

void ColorPanel::FillToolbarHistory()
{
    QString defaultStyle = toolbarButtonsDefaultStyle->BuildStylesheet();
    ui->toolbarHistoryColor1->setStyleSheet(defaultStyle);
    ui->toolbarHistoryColor2->setStyleSheet(defaultStyle);
    ui->toolbarHistoryColor3->setStyleSheet(defaultStyle);
    ui->toolbarHistoryColor4->setStyleSheet(defaultStyle);
    ui->toolbarHistoryColor5->setStyleSheet(defaultStyle);
    ui->toolbarHistoryColor6->setStyleSheet(defaultStyle);

    int historyCount = colorPickerHistory->history->count();
    for (int i = historyCount - 1; i >= 0; i--) {
        StylesheetHelper * buttonStyle = new StylesheetHelper();
        buttonStyle->SetRule(new QString("border-radius"), new QString("6px"));
        buttonStyle->SetRule(new QString("width"), new QString("32px"));
        buttonStyle->SetRule(new QString("height"), new QString("32px"));

        QColor currentColor = colorPickerHistory->history->at(i);
        buttonStyle->SetRule(new QString("background-color"), new QString(currentColor.name(QColor::HexRgb)));
        buttonStyle->SetRule(new QString("border"), new QString("1px solid " + currentColor.name(QColor::HexRgb)));
        int buttonIndex = (historyCount - 1) - i;
        switch (buttonIndex) {
            case 0:
                ui->toolbarHistoryColor1->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            case 1:
                ui->toolbarHistoryColor2->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            case 2:
                ui->toolbarHistoryColor3->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            case 3:
                ui->toolbarHistoryColor4->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            case 4:
                ui->toolbarHistoryColor5->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            case 5:
                ui->toolbarHistoryColor6->setStyleSheet(buttonStyle->BuildStylesheet());
            break;
            default:
                return;
        }
    }
}

void ColorPanel::ShowNotification(QString title, QString message)
{
    sysTray->showMessage(title, message, QSystemTrayIcon::Information, 2000);
}

QString ColorPanel::GetColorString(QColor color)
{
    int colorFormat = ui->colorFormatSelector->currentIndex();
    switch(colorFormat)
    {
        case 0: // HEX
            return color.name(QColor::HexRgb).toUpper();
        case 1: // RGB
        {
            QString str = "rgb(" +
            QString::number( color.red() ) + ", " +
            QString::number( color.green() ) + ", " +
            QString::number( color.blue() ) + ")";
            return str;
        }
        case 2: // HSV
        {
            QString str = "hsv(" +
            QString::number( color.hsvHue() ) + "º, " +
            QString::number( (int)round(((float)color.hsvSaturation() / (float)255) * (float)100) ) + "%, " +
            QString::number( (int)round(((float)color.value() / (float)255) * (float)100) ) + "%)";
            return str;
        }
        case 3: // HSL
        {
            QString str = "hsl(" +
            QString::number( color.hslHue() ) + ", " +
            QString::number( (int)round(((float)color.hslSaturation() / (float)255) * (float)100) ) + "%, " +
            QString::number( (int)round(((float)color.lightness() / (float)255) * (float)100) ) + "%)";
            return str;
        }
        case 4: // CMYK
        {
            QString str = "cmyk(" +
            QString::number( (int)round(((float)color.cyan() / (float)255) * (float)100) ) + "%, " +
            QString::number( (int)round(((float)color.magenta() / (float)255) * (float)100) ) + "%, " +
            QString::number( (int)round(((float)color.yellow() / (float)255) * (float)100) ) + "%, " +
            QString::number( (int)round(((float)color.black() / (float)255) * (float)100) ) + "%)";
            return str;
        }
        default:
            return color.name(QColor::HexRgb);
    }
}

void ColorPanel::on_stayOnTopButton_toggled(bool checked)
{
    if(checked)
    {
        setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        show();
    }
    else
    {
        setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
        show();
    }
}


void ColorPanel::on_clearHistoryButton_clicked()
{
    colorPickerHistory->Clear();
    FillHistory();
    FillToolbarHistory();
    colorPickerHistory->SaveHistory();
}

void ColorPanel::on_colorFormatSelector_currentIndexChanged(int index)
{
    settings->setValue("picker/colorformat", index);
}
