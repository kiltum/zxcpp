#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingswindow.h"
#include "emu.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    SettingsWindow *sw;

private slots:
    void on_actionSettings_triggered();

    void on_actionExit_triggered();

    void on_actionScale_1x_triggered();

    void on_actionScale_2x_triggered();

    void on_actionScale_3x_triggered();

    void screenUpdate(void);

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    QImage emuScreen;
    bool screenResising;
    Emu *emu;
    void Reconfigure(void);
    uint scaledWight;
    uint scaledHeight;
    void ScaleMainWindow(uint wi, uint he);
};
#endif // MAINWINDOW_H
