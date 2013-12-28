//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef RFACTION_H
#define RFACTION_H

#include <QAction>

class RFAction : public QAction
{
    Q_OBJECT
public:
    explicit RFAction(QString filename, QObject *parent = 0);
    void setfn(QString &nfn);
signals:
    
private slots:
    void iamclicked();
private:
    QString fn;
};

#endif // RFACTION_H
