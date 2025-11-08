#include <QSettings>

#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    settings = new QSettings();

    ui->turboTape->setChecked(true);
    ui->windowCrop->setChecked(true);

    ui->joystickSchema->setCurrentIndex(0);
    on_joystickSchema_currentIndexChanged(0);

    on_setAll48_clicked();

}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_buttonBox_accepted()
{

}


void SettingsWindow::on_buttonBox_rejected()
{

}


void SettingsWindow::on_setAll48_clicked() // set all setting to classic 48k
{
    ui->ramAmount->setCurrentIndex(0);
    ui->cpuSpeed->setCurrentIndex(0);
    ui->ulaTicks->setCurrentIndex(0);
    ui->rom0->setCurrentIndex(0);
    ui->bdiEnable->setCheckState(Qt::Unchecked);
    ui->enableAy->setCheckState(Qt::Unchecked);

}


void SettingsWindow::on_setAll128_clicked()
{
    ui->ramAmount->setCurrentIndex(1);
    ui->cpuSpeed->setCurrentIndex(1);
    ui->ulaTicks->setCurrentIndex(1);
    ui->rom0->setCurrentIndex(1);
    ui->rom1->setCurrentIndex(0);
    ui->bdiEnable->setCheckState(Qt::Checked);
    ui->enableAy->setCheckState(Qt::Checked);
    ui->bdiRom->setCurrentIndex(0);
}


void SettingsWindow::on_joystickSchema_currentIndexChanged(int index) // Set joystick schema
{
    switch (index) {
    case 0: // Cursor + Alt
        ui->joystickDown->setKeySequence(Qt::Key_Down);
        ui->joystickUp->setKeySequence(Qt::Key_Up);
        ui->joystickFire->setKeySequence(Qt::Key_Alt);
        ui->joystickLeft->setKeySequence(Qt::Key_Left);
        ui->joystickRight->setKeySequence(Qt::Key_Right);
        break;
    case 1: // Cursor + Tab
        ui->joystickDown->setKeySequence(Qt::Key_Down);
        ui->joystickUp->setKeySequence(Qt::Key_Up);
        ui->joystickFire->setKeySequence(Qt::Key_Tab);
        ui->joystickLeft->setKeySequence(Qt::Key_Left);
        ui->joystickRight->setKeySequence(Qt::Key_Right);
        break;
    default:
        break;
    }
}

void SettingsWindow::LoadSettings()
{

}

void SettingsWindow::WriteSettings()
{

}
