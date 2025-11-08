#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingswindow.h"

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

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent*);
    QImage emuScreen;
    bool screenResising;

};
#endif // MAINWINDOW_H
