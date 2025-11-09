#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"
#include "emu.h"

#include <QResizeEvent>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sw = new SettingsWindow;

    sw->LoadSettings();
    int x = sw->settings->value("main/x",200).toInt();
    int y = sw->settings->value("main/y",200).toInt();
    int wi = sw->settings->value("main/w",400).toInt();
    int he = sw->settings->value("main/h",400).toInt();

    setGeometry(x, y, wi, he);

    // Here is stub for future emu screen
    emuScreen = QImage(352, 288, QImage::Format_ARGB32);
    emuScreen.fill(Qt::green);
    QPainter painter(&emuScreen);
    QPen pen;
    pen.setColor(Qt::red); // Set the line color to red
    pen.setWidth(1);       // Set the line width to 2 pixels
    pen.setStyle(Qt::SolidLine); // Set the line style
    painter.setPen(pen);   // Apply the pen to the painter
    painter.drawLine(0, 144, 352, 144); // Draw cross
    painter.drawLine(176, 0, 176, 288);
    painter.end();

    QPixmap emuPix = QPixmap::fromImage(emuScreen);
    ui->emuWindow->setPixmap(emuPix);

    screenResising = true;

    emu = new Emu;
    Reconfigure();

    QObject::connect(emu, &Emu::updateScreen,this, &MainWindow::screenUpdate);
}

MainWindow::~MainWindow()
{
    sw->settings->setValue("main/x", geometry().x());
    sw->settings->setValue("main/y", geometry().y());
    sw->settings->setValue("main/w", geometry().width());
    sw->settings->setValue("main/h", geometry().height());

    sw->WriteSettings();
    delete ui;
}

void MainWindow::on_actionSettings_triggered()
{  
    sw->show();
}

void MainWindow::Reconfigure(void)
{
    switch(sw->cpu) {
    case 0: // zx 48
        emu->cpuSpeed = 3500000;
        emu->busDelimeter = 1;
        break;
    case 1: // zx 128
        emu->cpuSpeed = 3546900;
        emu->busDelimeter = 1;
        break;
    case 2:
        emu->cpuSpeed = 7000000;
        emu->busDelimeter = 2;
        break;
    case 3:
        emu->cpuSpeed = 14000000;
        emu->busDelimeter = 4;
        break;
    case 4:
        emu->cpuSpeed = 28000000;
        emu->busDelimeter = 8;
        break;
    default:
        emu->cpuSpeed = 3500000;
        emu->busDelimeter = 1;
        break;
    }

    switch (sw->ram) {
    case 0: // 48
        emu->setMemoryType(ZX_SPECTRUM_48);
        break;
    case 1: // 128
        emu->setMemoryType(ZX_SPECTRUM_128);
        break;
    default:
        break;
    }

    switch (sw->ula) {
    case 0: // 48
        emu->setULAType(ZX_SPECTRUM_48);
        break;
    case 1: // 128
        emu->setULAType(ZX_SPECTRUM_128);
        break;
    default:
        break;
    }

    emu->Reset();
    emu->Run();
    // Test. Screen must be black here
    //memcpy(emuScreen.bits(),emu->getScreenBuffer(),(352 * 288)*sizeof(uint32_t));
}

void MainWindow::screenUpdate(void)
{
   memcpy(emuScreen.bits(),emu->getScreenBuffer(),(352 * 288)*sizeof(uint32_t));
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if(screenResising) {
    QPixmap emuPix = QPixmap::fromImage(emuScreen.scaled(ui->centralwidget->width()-6,ui->centralwidget->height()-6,Qt::KeepAspectRatio));
    ui->emuWindow->setPixmap(emuPix);
    }
    screenResising = true;
    //printf("%d %d %d %d\n",s.width(), s.height(),ui->centralwidget->width(),ui->centralwidget->height());
    // QImage image(s.width(), s.height(), QImage::Format_ARGB32);
    // image.fill(Qt::blue);
    // QPixmap pixmap = QPixmap::fromImage(image);
    // ui->emuWindow->setPixmap(pixmap);

    //ui->emuWindow->setPixmap(emuPix);
    //ui->emuWindow->resize(350,250);
}


void MainWindow::on_actionScale_1x_triggered()
{
    QPixmap emuPix = QPixmap::fromImage(emuScreen);
    ui->emuWindow->setPixmap(emuPix);
    screenResising = false;
    QRect g = geometry();
    g.setWidth(emuPix.width());
    g.setHeight(emuPix.height());
    setGeometry(g);
}


void MainWindow::on_actionScale_2x_triggered()
{
    QPixmap emuPix = QPixmap::fromImage(emuScreen.scaled(emuScreen.width()*2,emuScreen.height()*2,Qt::KeepAspectRatio));
    ui->emuWindow->setPixmap(emuPix);
    screenResising = false;
    QRect g = geometry();
    g.setWidth(emuPix.width());
    g.setHeight(emuPix.height());
    setGeometry(g);
}


void MainWindow::on_actionScale_3x_triggered()
{
    QPixmap emuPix = QPixmap::fromImage(emuScreen.scaled(emuScreen.width()*3,emuScreen.height()*3,Qt::KeepAspectRatio));
    ui->emuWindow->setPixmap(emuPix);
    screenResising = false;
    QRect g = geometry();
    g.setWidth(emuPix.width());
    g.setHeight(emuPix.height());
    setGeometry(g);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    bool rightShift = false;
    if(event->key() == Qt::Key_Shift && event->nativeVirtualKey()==60) {
        rightShift = true;
    }
    if (!emu->mapKeyToSpectrum(event->key(), true, rightShift)) {
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    bool rightShift = false;
    if(event->key() == Qt::Key_Shift && event->nativeVirtualKey()==60) {
        rightShift = true;
    }
    if (!emu->mapKeyToSpectrum(event->key(), false, rightShift)) {
        QWidget::keyReleaseEvent(event);
    }
}
