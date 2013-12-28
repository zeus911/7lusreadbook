//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef BLOCK_H
#define BLOCK_H

#include <QMap>
#include <QObject>



class fileblock :public QObject
{
    Q_OBJECT
public:
    int length;
    qint64 id;
    bool available;
    char *buf;
    unsigned int code;
    QMap<int,bool> stops;
    fileblock(qint64 id);

    ~fileblock();
    
    void releaseMemory();
    void loadFile();
};

#endif // BLOCK_H
