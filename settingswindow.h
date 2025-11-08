#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();
    void LoadSettings(void);
    void WriteSettings(void);
    QSettings *settings;
    bool turboTape;
    bool windowCrop;
    int ram;
    int cpu;
    int ula;
    int rom0;
    int rom1;
    bool bdi;
    int bdiRom;
    bool ay;
    int joySchema;
    int joyAssign;
    QString joyLeft;
    QString joyRight;
    QString joyUp;
    QString joyDown;
    QString joyFire;



private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_setAll48_clicked();

    void on_setAll128_clicked();

    void on_joystickSchema_currentIndexChanged(int index);

private:

    Ui::SettingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
