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
