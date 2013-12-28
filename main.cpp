//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
MainWindow *w;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*QTranslator qtrans;
    qtrans.load("qt_zh_CN.qm");
    a.installTranslator(&qtrans);*/
    w=new MainWindow();
    w->show();
    
    return a.exec();
}
