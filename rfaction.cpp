//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "rfaction.h"
#include "mainwindow.h"
extern MainWindow *w;

RFAction::RFAction(QString filename,QObject *parent) :
    QAction(parent)
{
    fn=filename;
    this->setText(fn);
    connect(this,SIGNAL(triggered()),this,SLOT(iamclicked()));
}

void RFAction::iamclicked()
{
    w->loadFile(fn);
}

void RFAction::setfn(QString &nfn)
{
    fn=nfn;
    this->setText(fn);
}
