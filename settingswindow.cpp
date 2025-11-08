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
    turboTape = true;
    ui->windowCrop->setChecked(true);
    windowCrop = true;

    cpu = 0;
    ram = 0;
    ula = 0;
    rom0 = 0;
    rom1 = 0;
    bdi = false;
    bdiRom = 0;
    ay = false;

    joySchema = 0;
    joyAssign = 0;

    joyLeft = "";
    joyRight = "";
    joyUp = "";
    joyDown = "";
    joyFire = "";

    ui->joystickSchema->setCurrentIndex(0);
    on_joystickSchema_currentIndexChanged(0);

    on_setAll48_clicked();

}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}


void SettingsWindow::on_buttonBox_rejected()
{
    LoadSettings();
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

void SettingsWindow::on_buttonBox_accepted()
{
    ram = ui->ramAmount->currentIndex();
    cpu = ui->cpuSpeed->currentIndex();
    turboTape = ui->turboTape->isChecked();
    windowCrop = ui->windowCrop->isChecked();
    ula = ui->ulaTicks->currentIndex();
    rom0 = ui->rom0->currentIndex();
    rom1 = ui->rom1->currentIndex();
    bdi = ui->bdiEnable->isChecked();
    bdiRom = ui->bdiRom->currentIndex();
    ay = ui->enableAy->isChecked();
    joySchema = ui->joystickSchema->currentIndex();
    joyAssign = ui->joystickType->currentIndex();

    joyLeft = ui->joystickLeft->keySequence().toString();
    joyRight = ui->joystickRight->keySequence().toString();
    joyUp = ui->joystickUp->keySequence().toString();
    joyDown = ui->joystickDown->keySequence().toString();
    joyFire = ui->joystickFire->keySequence().toString();
}

void SettingsWindow::LoadSettings()
{
    ram = settings->value("emu_ram",0).toInt();
    if(ram < 0 || ram > 1 ) ram = 0;
    ui->ramAmount->setCurrentIndex(ram);

    cpu = settings->value("emu_cpu",0).toInt();
    if(cpu < 0 || cpu > 4 ) cpu = 0;
    ui->cpuSpeed->setCurrentIndex(cpu);

    turboTape = settings->value("emu_turbotape",true).toBool();
    if(turboTape)
    ui->turboTape->setCheckState(Qt::Checked);
    else
    ui->turboTape->setCheckState(Qt::Unchecked);

    windowCrop = settings->value("emu_windowcrop",true).toBool();
    if(turboTape)
        ui->windowCrop->setCheckState(Qt::Checked);
    else
        ui->windowCrop->setCheckState(Qt::Unchecked);

    ula = settings->value("emu_ula",0).toInt();
    if(ula < 0 || ula > 1 ) ula = 0;
    ui->ulaTicks->setCurrentIndex(ula);

    rom0 = settings->value("emu_rom0",0).toInt();
    if(rom0 < 0 || rom0 > 3 ) rom0 = 0;
    ui->rom0->setCurrentIndex(rom0);

    rom1 = settings->value("emu_rom1",0).toInt();
    if(rom1 < 0 || rom1 > 0 ) rom1 = 0;
    ui->rom1->setCurrentIndex(rom1);

    bdi = settings->value("emu_bdi",true).toBool();
    if(bdi)
        ui->bdiEnable->setCheckState(Qt::Checked);
    else
        ui->bdiEnable->setCheckState(Qt::Unchecked);

    bdiRom = settings->value("emu_bdi_rom",0).toInt();
    if(bdiRom < 0 || bdiRom > 2 ) bdiRom = 0;
    ui->bdiRom->setCurrentIndex(bdiRom);

    ay = settings->value("emu_ay",true).toBool();
    if(ay)
        ui->enableAy->setCheckState(Qt::Checked);
    else
        ui->enableAy->setCheckState(Qt::Unchecked);


    joySchema = settings->value("emu_joy_schema",0).toInt();
    if(joySchema < 0 || joySchema > 1 ) joySchema = 0;
    ui->joystickSchema->setCurrentIndex(joySchema);

    joyAssign = settings->value("emu_joy_assign",0).toInt();
    if(joyAssign < 0 || joyAssign > 2 ) joyAssign = 0;
    ui->joystickType->setCurrentIndex(joyAssign);

    joyLeft = settings->value("emu_joy_left","").toString();
    ui->joystickLeft->setKeySequence(QKeySequence(joyLeft));

    joyRight = settings->value("emu_joy_right","").toString();
    ui->joystickRight->setKeySequence(QKeySequence(joyRight));

    joyUp = settings->value("emu_joy_up","").toString();
    ui->joystickUp->setKeySequence(QKeySequence(joyUp));

    joyDown = settings->value("emu_joy_down","").toString();
    ui->joystickDown->setKeySequence(QKeySequence(joyDown));

    joyFire = settings->value("emu_joy_fire","").toString();
    ui->joystickFire->setKeySequence(QKeySequence(joyFire));

}

void SettingsWindow::WriteSettings()
{
    settings->setValue("emu_cpu", cpu);
    settings->setValue("emu_ram", ram);
    settings->setValue("emu_ula", ula);
    settings->setValue("emu_turbotape",turboTape);
    settings->setValue("emu_windowcrop",windowCrop);
    settings->setValue("emu_rom0", rom0);
    settings->setValue("emu_rom1", rom1);
    settings->setValue("emu_bdi",bdi);
    settings->setValue("emu_bdi_rom",bdiRom);
    settings->setValue("emu_ay",ay);
    settings->setValue("emu_joy_schema",joySchema);
    settings->setValue("emu_joy_assign",joyAssign);
    settings->setValue("emu_joy_left", joyLeft);
    settings->setValue("emu_joy_right", joyRight);
    settings->setValue("emu_joy_up", joyUp);
    settings->setValue("emu_joy_down", joyDown);
    settings->setValue("emu_joy_fire", joyFire);
}
