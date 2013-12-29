//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "block.h"
#include "mainwindow.h"
#include <QFile>
#include <QHash>

extern unsigned int stopcode,bomsize,currentblocks;
extern QMap<qint64,fileblock*> blocks;
extern QFile *file;
extern qint64 realfilesize;


fileblock::fileblock(qint64 id):
    QObject()
{
    this->id=id;
    code=0;
    loadFile();
    if(available)
    {
        blocks.insert(id,this);
    }
}

fileblock::~fileblock()
{
    if(available)
    {
        delete[] buf;
        available=false;
        currentblocks--;
    }
    stops.clear();
}

void fileblock::releaseMemory()
{
    if(available)
    {
        delete[] buf;
        available=false;
        currentblocks--;
    }
    if(code!=stopcode)
    {
        blocks.remove(id);
        this->deleteLater();
    }
}

void fileblock::loadFile()
{
    qint64 seek;
    buf=new char[POP_BLOCKSIZE];
    seek=id*65536+bomsize;
    file->seek(seek);
    length=file->read(buf,POP_BLOCKSIZE);
    if(length==-1)
    {
        available=false;
        delete[] buf;
    }
    else
    {
        available=true;
        currentblocks++;
    }
}
