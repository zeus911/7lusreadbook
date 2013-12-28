//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "textout.h"
#include <QTextDecoder>
#include <QSettings>
#include "mainwindow.h"
#include "ui_mainwindow.h"

extern QHash<qint64,fileblock*> blocks;
extern QFile *file;
extern QTextCodec *codec;
extern qint64 filepos,maxblock,realfilesize;
extern QSettings *settings;
extern unsigned int stopcode;

textOut::textOut(QWidget *parent) :
    QWidget(parent)
{
    int fontr,fontg,fontb,bgr,bgg,bgb;
    int fontsize,fontweight;
    bool italic;
    QString fontfamily;
    fontr=settings->value("_fontr",127).toInt();
    fontg=settings->value("_fontg",255).toInt();
    fontb=settings->value("_fontb",255).toInt();
    bgr=settings->value("_bgr",0).toInt();
    bgg=settings->value("_bgg",0).toInt();
    bgb=settings->value("_bgb",0).toInt();
    fontfamily=settings->value("_fontfamily","Sans").toString();
    fontsize=settings->value("_fontsize",30).toInt();
    fontweight=settings->value("_fontweight",50).toInt();
    italic=settings->value("_fontitalic",false).toBool();
    
    guessMode=0;

    font=new QFont(fontfamily, fontsize,fontweight,italic);
    fontcolor=new QPen(QColor(fontr,fontg,fontb));
    fontm=new QFontMetrics(*font);
    background=new QBrush(QColor(bgr,bgg,bgb));
    cache=NULL;
    needpiece=-1;
    //fontspace=1.0;
    //font->setLetterSpacing(QFont::AbsoluteSpacing,fontspace);
    linespace=settings->value("_linespace",8.0).toReal();
    padding=settings->value("_padding",1.0).toReal();
    maxblockinmemory=settings->value("_maxblockinmemory",100).toInt();
}

//重载这个函数绝非我所愿，因为这会使代码量和编程难度都大幅增加，但因为找不到合我心意的控件（QPLAINTEXTEDIT不能很好地处理滚动条、位置判断方面的需要），只能自己进行输出
void textOut::paintEvent(QPaintEvent *eve)
{
    if(needrenew)//判断是不是要重绘，不用的话就直接读缓存
    {
        needrenew=false;
        painter.begin(cache);//写入缓存
        painter.setPen(*fontcolor);
        painter.setFont(*font);
        painter.fillRect(rect(),*background);
        if(file->isOpen())
        {
            /*变量说明：
             tpos是在块中要读取的位置
             x是输出位置的水平坐标
             y是输出位置的Y坐标
             H是每行的行高加上间距
             TAREA是要读取的块的序号
             TS是读取到并解码的字符串
             FB是当前的块的指针
             ISOVER判断是不是读满屏幕了

             decoder是解码器，把非UNICODE编码的文件转换为UNICODE存到TS中
             STOP是断行点MAP的迭代器
             READLENGTH是一次读取的长度

             endpos是记录多字节汉字的第一个字符的位置，如果在读第二个字节（或者第3个，看编码）的时候刚好要换行，那么在ENDPOS-1就会记录断行点。
             断行点是为了加速输出位置判断而设的（特别是向上翻的时候）
             本段为程序的核心部分之一，代码偏底层，较难理解，故加上注释
            */
            tpos=filepos%POP_BLOCKSIZE;
            qreal h,t;
            qreal width=this->width()-padding;
            h=fontm->height()+linespace;
            tarea=filepos/POP_BLOCKSIZE;
            

            bool isover=false;
            //bool hasnewline=false;
            QTextDecoder decoder(codec);

            int readlength;
            int endpos=-1;
            bool recorded,neednewline;
            QPointF outpos(padding,h);
            QMap<int,bool>::iterator tstop;
            recorded=false;
            
aa:         if(blockAvailable(tarea))//判断需要的区块是不是已经读入内存了
            {
                fb=blocks[tarea];
                if(fb->code!=stopcode)//这个CODE是用来判断断行点是否有效，字体变化、WIDGET的宽度变化都会导致STOPCODE变化从而使原来的断行点失效。
                {
                    fb->code=stopcode;
                    fb->stops.clear();
                }
                while(tpos<fb->length)
                {
                    neednewline=false;
                    if(outpos.x()==padding)//是不是行的开头
                    {
                        if(!recorded)//是否已记录
                        {
                            recorded=true;
                            stop=fb->stops.find(tpos);//判断是否存在已记录的断行点
                            if(stop!=fb->stops.end())
                            {
                                if(stop.value())//是否是有结尾的断行点
                                {
                                    tstop=stop+1;
                                    if(tstop==fb->stops.end())//块尾判断
                                    {
                                        readlength=fb->length-stop.key();
                                    }
                                    else
                                    {
                                        readlength=tstop.key()-tpos;
                                        neednewline=true;
                                    }
                                }
                                else//无需记录断行点
                                {
                                    readlength=1;
                                }
                            }
                            else//没有断行点需要记录一个无结尾的断行点
                            {
                                stop=fb->stops.insert(tpos,false);
                                readlength=1;
                            }
                        }
                        else//这是没有断行点的时候进行单步读取的情况
                        {
                            readlength=1;
                        }
                    }
                    else//这也是没有断行点的时候进行单步读取的情况
                    {
                        readlength=1;
                    }
                    ts=decoder.toUnicode(fb->buf+tpos,readlength);
                    tpos+=readlength;
                    if(ts=="")//如果读出来的数据是空的话就跳过，通常对于2字节的汉字来说，读第一个字节的时候就会是这种情况。DECODER会自动记录第1个字节，读到第2个字节的时候就会解码出完整的汉字来。
                    {
                        if(endpos==-1)
                        {
                            endpos=tpos-readlength;
                        }
                        continue;
                    }
                    if(ts=="\n")
                    {
                        //if(!hasnewline)
                        //{
                        //因为想不到好的代码重用的办法，只好直接用跳转了。
bb:                     tstop=fb->stops.find(tpos);
                        if(tstop==fb->stops.end())
                        {
                            tstop=fb->stops.insert(tpos,false);//先判断再插是为了不改变原有的值（如果有的话）
                        }
                        if(recorded&&tstop!=stop)
                        {
                            while(tstop!=fb->stops.begin())//清掉中间的断行点，可能是guessmode下的产物。
                            {
                                if(--tstop==stop)
                                {
                                    break;
                                }
                                tstop=fb->stops.erase(tstop);
                            }
                            stop.value()=true;
                        }
cc:                     if(outpos.y()>=this->height())//如果现在的行是屏幕所能显示的最后一行，那么结束绘制
                        {
                            isover=true;
                            /*t=tpos;
                            while(t<fb->length)
                            {
                                if(fb->buf[tpos]!='\n')
                                {*/
                            
                                /*}
                                t++;
                            }*/
                            break;
                        }
                        outpos.rx()=padding;
                        outpos.ry()+=h;
                        
                        //hasnewline=true;
                        //}
                        endpos=-1;
                        recorded=false;
                        continue;
                    }
                    else if(ts=="\r")
                    {
                        endpos=-1;
                        continue;
                    }
                    
                    /*if(hasnewline)
                    {
                        if(linefirst!=-1&&linefirst!=tpos-readlength)
                        {
                            fb->stops.insert(linefirst,tpos-linefirst-readlength);
                        }
                        linefirst=tpos;
                        hasnewline=false;
                    }*/
                    if(ts.endsWith('\n'))
                    {
                        ts.remove(ts.length()-1,1);
                    }
                    if(ts.endsWith('\r'))
                    {
                        ts.remove(ts.length()-1,1);
                    }
                    if(neednewline)//如果是整行读入的话
                    {
                        painter.drawText(outpos,ts);
                        goto cc;//直接换新行
                    }
                    
                    t=fontm->width(ts);
                    if(t>width-outpos.x())//要换行了
                    {
                        if(endpos!=-1)
                        {
                            tpos=endpos;
                        }
                        else
                        {
                            tpos--;
                        }
                        goto bb;
                    }
                    painter.drawText(outpos,ts);
                    outpos.rx()+=t;
                    endpos=-1;
                }
                if(!isover)
                {
                    tarea++;
                    if(tarea<=maxblock)
                    {
                        tpos=0;
                        goto aa;
                    }
                    else
                    {
                        if(recorded)
                        {
                            tstop=stop+1;
                            while(tstop!=fb->stops.end())//文件尾清除不正确断行点
                            {
                                tstop=fb->stops.erase(tstop);
                            }
                            stop.value()=true;
                        }
                    }
                }
            }
        }
        else
        {
            painter.drawText(rect(),Qt::AlignCenter|Qt::TextWordWrap, tr("欢迎使用奇路小说阅读器！"));
        }
        
        painter.end();
    }
    painter.begin(this);
    painter.drawPixmap(eve->rect(),cache->copy(eve->rect()));//读缓存
    painter.end();
}

void textOut::renewFontInfo()
{
    if(fontm)
    {
        delete fontm;
    }
    fontm=new QFontMetrics(*font);
}

void textOut::resizeEvent(QResizeEvent *eve)
{
    if(cache)
    {
        delete cache;
    }
    cache=new QPixmap(eve->size());
    needrenew=true;
}

bool textOut::lineMove(bool up,bool ifupdate)
{
    if(!file->isOpen())
    {
        return false;
    }
    tarea=filepos/POP_BLOCKSIZE;
    tpos=filepos%POP_BLOCKSIZE;
    int i,t1,t2,max;
    bool success1,success2;
    fileblock *tfb;
    
    qreal width,x;
    //bool flag=true;
    if(blockAvailable(tarea))
    {
        fb=blocks[tarea];
        if(fb->code!=stopcode)
        {
            fb->code=stopcode;
            fb->stops.clear();
        }
    }
    else
    {
        return false;
    }
    
    if(up)//是上滚还是下滚
    {
        if(filepos!=0)
        {
            if(guessMode>0)
            {
                guessMode--;
                goto dd;
            }
            success2=false;
            stop=fb->stops.find(tpos);
            if(stop!=fb->stops.end()&&stop!=fb->stops.begin())
            {
                success2=true;
                stop--;
                if(stop.value()==true)//有可用的断行点
                {
                    filepos+=stop.key()-tpos;
                    return true;
                }
            }
            success1=false;
            //不灰心，看看是不是可以利用前面的区块的断行点
            if(success2)
            {
                t1=stop.key();
                max=tpos-t1;
                if(max<4096)//上一个无结尾的断行点离这里不远
                {
                    success1=true;
                }
            }
            else if(tpos<4095)
            {
                if(blockAvailable(tarea-1))//检查上一个区块是不是有近的断行点
                {
                    tfb=blocks[tarea-1];
                    if(tfb->code!=stopcode)//使用断行点前记得检查是否有效
                    {
                        tfb->code=stopcode;
                        tfb->stops.clear();
                    }
                    else
                    {
                        if(tfb->stops.size()>0)
                        {
                            stop=tfb->stops.end();
                            stop--;
                            if(POP_BLOCKSIZE-stop.key()<4095)
                            {
                                t1=stop.key();
                                max=POP_BLOCKSIZE-t1+tpos;
                                if(max<4096)//可用
                                {
                                    success1=true;
                                }
                            }
                        }
                    }
                }
            }
            
            if(!success1)
            {
                max=4096;
            }
            
            success2=false;
            tfb=fb;
            t2=tpos;
            for(i=0;i<max;i++)//往前找找，看有没有换行符，如果两个都成功的话，选少的那个
            {
                t2--;
                if(t2<0)
                {
                    if(tarea==0)//遇到文件头了
                    {
                        t2=0;
                        success2=true;
                        break;
                    }
                    t2=POP_BLOCKSIZE-1;
                    if(!blockAvailable(tarea-1))
                    {
                        return false;
                    }
                    tfb=blocks[tarea-1];
                }
                if(tfb->buf[t2]=='\n')//找到换行符了
                {
                    if(i!=0)//只要不是第一个字符
                    {
                        t2++;
                        if(t2==POP_BLOCKSIZE)//如果刚好是在块尾找到的
                        {
                            tfb=fb;
                            t2=0;
                        }
                        success2=true;
                        break;
                    }
                }
            }
            
            if(success1||success2)//从找到的位置开始画，并设置断行点，就找到最后一行的位置了
            {
                if(success2)
                {
                    max=i;
                    t1=t2;
                }
                if(max>tpos)
                {
                    tfb=blocks[tarea-1];
                }
                else
                {
                    tfb=fb;
                }
                QTextDecoder decoder(codec);
                width=this->width()-padding*2;
                x=0;
                int tw,whitecount=0;
                stop=tfb->stops.insert(t1,false);
                for(i=0;i<max;i++)//当然不必真画，只要算距离就好了
                {
                    if(i==max-1)
                    {
                        if(tfb->buf[t1]=='\n')
                        {
                            break;
                        }
                    }
                    decoder.toUnicode(&ts,tfb->buf+t1,1);
                    if(ts=="")
                    {
                        whitecount++;
                    }
                    else
                    {
                        tw=fontm->width(ts);
                        if(tw>width-x)//一个一个字符的计算宽度，满了就换行
                        {
                            x=tw;
                            if(stop.key()<t1)
                            {
                                stop.value()=true;
                            }
                            stop=tfb->stops.insert(t1-whitecount,false);//断行点设好了，等下再有上滚指令的话就很快了。
                        }
                        else
                        {
                            x+=tw;
                        }
                        whitecount=0;
                    }
                    t1++;
                    if(t1==POP_BLOCKSIZE)
                    {
                        tfb=fb;
                        t1=0;
                    }
                }
                if(stop.key()<t1)
                {
                    stop.value()=true;
                }
                if(stop.key()>tpos)
                {
                    filepos-=tpos+(POP_BLOCKSIZE-stop.key());
                }
                else
                {
                    filepos-=tpos-stop.key();
                }
                return true;
            }
            else//找了4k都没找到换行符，更苦逼了，只能猜了。
            {
                guessMode=10;
                if(!ifupdate) return false;//如果是翻页的过程的话，直接就不找了
dd:             width=this->width()-padding*2;
                QTextDecoder decoder(codec);
                x=0;
                int singlewidth,mpos,l;
                QChar qc;
                singlewidth=fontm->width("我");
                t1=width/singlewidth+1;
                t1*=3;
                char* tbuf=new char[t1];
                mpos=tpos;
                max=0;
                while(true)//不找到填满内容的一行不回头
                {
                    mpos-=t1;
                    t2=readBuf(tbuf,tarea,mpos,t1);
                    if(!t2)
                    {
                        decoder.toUnicode(&ts,tbuf,t1);
                        l=ts.length();
                        for(i=l-1;i>=0;i--)
                        {
                            qc=ts[i];
                            if(qc.toLatin1()==0)
                            {
                                x+=singlewidth;
                                t2=2;
                            }
                            else
                            {
                                x+=fontm->width(qc);
                                t2=1;
                            }
                            if(x>width)//结束了
                            {
                                filepos-=max;
                                delete[] tbuf;
                                return true;
                            }
                            max+=t2;
                        }
                    }
                    else if(t2==POP_NOTINMEMORY)
                    {
                        delete[] tbuf;
                        return false;
                    }
                    else
                    {
                        guessMode=0;
                        filepos=0;
                        delete[] tbuf;
                        return true;
                    }
                }
            }
        }
        else
        {
            return true;
        }
    }
    else
    {
        stop=fb->stops.find(tpos);
        if(stop!=fb->stops.end()&&stop.value()==true)
        {
            stop++;
            if(stop==fb->stops.end())//块尾
            {
                if(tarea==maxblock)
                {
                    return true;
                }
                if(blockAvailable(tarea+1))
                {
                    fb=blocks[tarea+1];
                    stop=fb->stops.lowerBound(0);
                    if(stop==fb->stops.end())
                    {
                        return false;
                    }
                    else
                    {
                        filepos+=stop.key()+(POP_BLOCKSIZE-tpos);
                        return true;
                    }
                }
            }
            filepos+=stop.key()-tpos;
            return true;
        }
    }
    return false;
}

bool textOut::blockAvailable(qint64 area)
{
    if(area<0||area>maxblock)
    {
        return false;
    }
    fileblock *nfb;
    if(!blocks.contains(area))
    {
        nfb=new fileblock(area);
        if(nfb->available)
        {
            blocks.insert(area,nfb);
            return true;
        }
        else
        {
            return false;
        }
    }
    nfb=blocks[area];
    if(nfb->available)
        return true;
    else
    {
        nfb->loadFile();
        if(nfb->available)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

int textOut::readBuf(char *out, qint64 blockid, int pos,int length)//无视界限地读取块的数据，不过因为效率不如直接读块的数据，暂时不用
{
    
    int a,t;
    a=pos/POP_BLOCKSIZE;
    blockid+=a;
    if(blockid>maxblock||blockid<0)
    {
        return POP_OUTOFRANGE;
    }
    t=pos%POP_BLOCKSIZE;
    if(!blockAvailable(blockid))
    {
        return POP_NOTINMEMORY;
    }
    pos=t+length;
    a=pos/POP_BLOCKSIZE;
    if(a!=0)
    {
        if(blockid+a>maxblock)
        {
            return POP_OUTOFRANGE;
        }
    }
    fb=blocks[blockid];
    a=0;
    while(true)
    {
        if(fb->length-t>=length)
        {
            memcpy(out+a,fb->buf+t,length);
            return 0;
        }
        else
        {
            pos=a;
            a+=fb->length-t;
            length-=a;
            memcpy(out+pos,fb->buf+t,length);
            blockid++;
            if(!blockAvailable(blockid))
            {
                return POP_NOTINMEMORY;
            }
            fb=blocks[blockid];
            if(blockid==maxblock&&length>fb->length)
            {
                return POP_OUTOFRANGE;
            }
            t=0;
        }
    }
}

bool textOut::pageMove(bool up)
{
    if(!file->isOpen())
    {
        return false;
    }
    int linecount,i;
    qreal height;
    qint64 tfpos=filepos;
    height=fontm->height()+linespace;
    linecount=this->height()/height-1;
    if(linecount<=0) linecount=1;
    if(up)
    {
        bool flag=true;
        if(guessMode>0)
        {
            guessMode-=3;
        }
        else
        {
            for(i=0;i<linecount;i++)
            {
                if(!lineMove(true,false))
                {
                    if(guessMode>0)
                    {
                        flag=false;
                        linecount-=i;
                        break;
                    }
                    else
                    {
                        filepos=tfpos;
                        return false;
                    }
                }
            }
            if(flag)
            {
                return true;
            }
        }
        //往上翻linecount行，把LINEMOVE的代码复制过来改一改
        qreal width=this->width()-padding*2;
        QTextDecoder decoder(codec);
        qreal x=0,rx;
        int singlewidth,mpos,l;
        QChar qc;
        int t1,t2,max;
        singlewidth=fontm->width("我");
        t1=width/singlewidth+1;
        t1*=3*linecount;
        char* tbuf=new char[t1];
        mpos=tpos;
        max=0;
        while(linecount>0)
        {
            mpos-=t1;
            t2=readBuf(tbuf,tarea,mpos,t1);
            if(!t2)
            {
                decoder.toUnicode(&ts,tbuf,t1);
                l=ts.length();
                for(i=l-1;i>=0;i--)
                {
                    qc=ts[i];
                    if(qc.toLatin1()==0)
                    {
                        rx=singlewidth;
                        x+=rx;
                        t2=2;
                    }
                    else
                    {
                        rx=fontm->width(qc);
                        x+=rx;
                        t2=1;
                    }
                    if(x>width)//结束一行
                    {
                        linecount--;
                        x=rx;
                        continue;
                    }
                    max+=t2;
                }
            }
            else if(t2==POP_NOTINMEMORY)
            {
                delete[] tbuf;
                return false;
            }
            else
            {
                guessMode=0;
                filepos=0;
                delete[] tbuf;
                return true;
            }
        }
        filepos-=max;
        delete[] tbuf;
        return true;
        //end of copy
    }
    else
    {
        for(i=0;i<linecount;i++)//往下翻还是很好写的，直接利用断行点不会有错
        {
            if(!lineMove(false,false))
            {
                filepos=tfpos;
                return false;
            }
        }
        return true;
    }
}


bool textOut::randomMove(qint64 newvalue)
{
    tarea=newvalue/POP_BLOCKSIZE;
    if(blockAvailable(tarea))
    {
        QMap<int,bool>::iterator tstop;
        int t1,t2,i,p;
        fileblock *tfb;
        p=tpos=newvalue%POP_BLOCKSIZE;
        tfb=fb=blocks[tarea];
        if(fb->code!=stopcode)
        {
            fb->code=stopcode;
            fb->stops.clear();
        }
        stop=fb->stops.lowerBound(tpos);
        if(stop!=fb->stops.end())
        {
            t1=stop.key()-tpos;
            if(t1>POP_MAXSEARCH)
            {
                t1=POP_MAXSEARCH;
            }
        }
        else
        {
            t1=POP_MAXSEARCH;
        }
        if(stop!=fb->stops.begin())
        {
            tstop=stop-1;
            t2=tpos-tstop.key();
            if(t2>POP_MAXSEARCH)
            {
                t2=POP_MAXSEARCH;
            }
        }
        else
        {
            t2=POP_MAXSEARCH;
        }
        if(t1!=POP_MAXSEARCH&&t2!=POP_MAXSEARCH)
        {
            if(t2<t1)
            {
                t1=t2;
            }
            else
            {
                t2=t1;
            }
        }
        else if(t2!=POP_MAXSEARCH)
        {
            t1=t2;
        }
        else if(t1!=POP_MAXSEARCH)
        {
            t2=t1;
        }
        for(i=0;i<t2;i++)
        {
            p--;
            if(p<0)
            {
                if(tarea==0)
                {
                    break;
                }
                if(!blockAvailable(tarea-1))
                {
                    return false;
                }
                tfb=blocks[tarea-1];
                p=POP_BLOCKSIZE-1;
            }
            if(tfb->buf[p]=='\n')
            {
                break;
            }
        }
        bool isfind=false;
        if(i<t2)
        {
            t2=i;
            isfind=true;
            if(t2<t1)
            {
                t1=t2;
            }
        }
        tfb=fb;
        p=tpos;
        for(i=0;i<t1;i++)
        {
            if(p>=tfb->length)
            {
                if(tarea==maxblock)
                {
                    i=t1;
                    break;
                }
                if(!blockAvailable(tarea+1))
                {
                    return false;
                }
                tfb=blocks[tarea+1];
                p=0;
            }
            if(tfb->buf[p]=='\n')
            {
                break;
            }
            p++;
        }
        if(i<t1)
        {
            t1=i+1;
        }
        if(t1==t2)
        {
            if(t1==POP_MAXSEARCH)
            {
                guessMode=10;
                filepos=newvalue;
                if(newvalue>=realfilesize-1)
                {
                    lineMove(true);
                }
                return true;
            }
            if(isfind)
            {
                t1=-t2;
            }
        }
        else if(t1>t2)
        {
            t1=-t2;
        }
        filepos=newvalue+t1;
        if(filepos>=realfilesize-1)
        {
            lineMove(true);
        }
        return true;
    }
    return false;
}
