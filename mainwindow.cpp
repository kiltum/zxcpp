#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"

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
    //printf("R %d %d %d %d\n",x,y,wi,he);

    setGeometry(x, y, wi, he);
}

MainWindow::~MainWindow()
{
    sw->settings->setValue("main/x", geometry().x());
    sw->settings->setValue("main/y", geometry().y());
    sw->settings->setValue("main/w", geometry().width());
    sw->settings->setValue("main/h", geometry().height());

    //printf("W %d %d %d %d\n",x(),y(),width(),height());
    //printf("W %d %d %d %d\n",geometry().x(),geometry().y(),geometry().width(),geometry().height());
    sw->WriteSettings();
    delete ui;
}

void MainWindow::on_actionSettings_triggered()
{  
    sw->show();
}


void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

