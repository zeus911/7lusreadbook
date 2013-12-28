//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFile>
#include <QTextLayout>
#include <QPainter>
#include <QTextStream>
#include "rfaction.h"
#include "dialogabout.h"
#include "dialogoptions.h"

#define POP_BLOCKSIZE 65536



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    Ui::MainWindow *ui;
    void loadFile(QString &fn);
    
    ~MainWindow();
    
private:
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *eve);
    void resizeEvent(QResizeEvent * eve);
    
    void setScrollBarPosition();
    QStringList recentFile;
    void closeEvent(QCloseEvent *);
    QString filename;
    RFAction* menuRF[10];
    DialogAbout *DAbout;
    DialogOptions *DOptions;

private slots:
    void menu_open();
    void menu_exit();

    void on_verticalScrollBar_actionTriggered(int action);
    void on_verticalScrollBar_sliderReleased();
    /*void menuGBK();
    void menuUnicode();
    void menuUnicodeBE();
    void menuUTF8();
    void menuBIG5();*/
    void menuCodec();
    void menuAbout();
    void menuOptions();
};

#endif // MAINWINDOW_H
