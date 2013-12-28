//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef TEXTOUT_H
#define TEXTOUT_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include "block.h"

#define POP_OUTOFRANGE 1
#define POP_NOTINMEMORY 2
#define POP_MAXSEARCH 4096

class textOut : public QWidget
{
    Q_OBJECT
public:
    explicit textOut(QWidget *parent = 0);
    void paintEvent(QPaintEvent *eve);
    QFont *font;
    QPen *fontcolor;
    QFontMetrics *fontm;
    QBrush *background;
    QPainter painter;
    QPixmap *cache;
    bool needrenew;
    void renewFontInfo();
    void resizeEvent(QResizeEvent *);
    qint64 needpiece;
    //qreal fontspace;
    qreal linespace;
    qreal padding;
    int maxblockinmemory;

    bool lineMove(bool up, bool ifupdate=true);
    bool pageMove(bool up);
    bool randomMove(qint64 newvalue);
private:
    bool blockAvailable(qint64 area);
    qint64 tarea;
    fileblock *fb;
    QMap<int,bool>::iterator stop;
    int tpos;
    QString ts;
    int readBuf(char* out, qint64 blockid, int pos, int length);
    int guessMode;
signals:
    
public slots:
    
};

#endif // TEXTOUT_H
