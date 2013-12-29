//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QLayout>
#include "block.h"
#include <QAbstractSlider>
#include <set>
QMap<qint64,fileblock*> blocks;
QFile *file;
QTextCodec *codec;
qint64 filepos,maxblock,realfilesize;
QSettings *settings;
unsigned int stopcode=1,bomsize=0,currentblocks=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    DAbout=NULL;
    DOptions=NULL;
    settings=new QSettings("7lus","Readbook");
    this->setWindowState(Qt::WindowMaximized);
    ui->setupUi(this);
    
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(menu_open()));
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(menu_exit()));
    connect(ui->actionGBK,SIGNAL(triggered()),this,SLOT(menuCodec()));
    connect(ui->actionUnicode,SIGNAL(triggered()),this,SLOT(menuCodec()));
    connect(ui->actionUnicode_Big_Endian,SIGNAL(triggered()),this,SLOT(menuCodec()));
    connect(ui->actionUTF_8,SIGNAL(triggered()),this,SLOT(menuCodec()));
    connect(ui->actionBIG5,SIGNAL(triggered()),this,SLOT(menuCodec()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(menuAbout()));
    connect(ui->actionOptions,SIGNAL(triggered()),this,SLOT(menuOptions()));

    
    file=new QFile();

    recentFile=settings->value("_recentfiles",NULL).toStringList();
    int i,l;
    l=recentFile.length();
    for(i=0;i<l;i++)
    {
        menuRF[i]=new RFAction(recentFile[i],this);
        ui->menuFile->insertAction(ui->actionExit,menuRF[i]);
    }
    if(l>0)
    {
        ui->menuFile->insertSeparator(ui->actionExit);
        filename=recentFile[0];
        loadFile(filename);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *eve)
{
    QSize size=this->centralWidget()->size();
    int width,height;
    width=size.width()-16;
    height=size.height();
    ui->verticalScrollBar->move(width,0);
    ui->verticalScrollBar->resize(16,height);
    if(width!=ui->widget->width())
    {
        stopcode++;
    }
    ui->widget->resize(width,height);
}

void MainWindow::menu_open()
{
    if(filename==""&&recentFile.size()>0)
    {
        filename=recentFile[0];
    }
    QString fn=QFileDialog::getOpenFileName(this,tr("打开文件"),filename,tr("文本文件(*.txt);;所有文件(*)"));
    if(fn!="")
    {
        filename=fn;
        loadFile(fn);
    }
}

void MainWindow::menu_exit()
{
    this->close();
}

void MainWindow::loadFile(QString &fn)
{
    if(file->isOpen())
    {
        settings->setValue(file->fileName(),filepos);
        QMap<qint64,fileblock*>::iterator block;
        block=blocks.begin();
        while(block!=blocks.end())
        {
            block.value()->deleteLater();
            block++;
        }
        blocks.clear();
        file->close();
        currentblocks=0;
    }
    recentFile.prepend(fn);
    file->setFileName(fn);
    if(!file->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,tr("错误"),tr("无法打开文件。"));
        return;
    }
    //判断编码方式（有BOM用BOM 没BOM就蒙吧）
    char bom[3];
    QList<QAction *> codecs=ui->menuCodec->actions();
    QAction* op;
    foreach (op, codecs) {
        op->setChecked(false);
    }
    if(file->read(bom,3)<3)
    {
        codec=QTextCodec::codecForName("GBK");
        ui->actionGBK->setChecked(true);
        bomsize=0;
    }
    else
    {
        if(!memcmp(bom,"\xef\xbb\xbf",3))
        {
            codec=QTextCodec::codecForName("UTF-8");
            ui->actionUTF_8->setChecked(true);
            bomsize=3;
        }
        else if(!memcmp(bom,"\xff\xfe",2))
        {
            codec=QTextCodec::codecForName("UTF-16LE");
            ui->actionUnicode->setChecked(true);
            bomsize=2;
        }
        else if(!memcmp(bom,"\xfe\xff",2))
        {
            codec=QTextCodec::codecForName("UTF-16BE");
            ui->actionUnicode_Big_Endian->setChecked(true);
            bomsize=2;
        }
        else
        {
            if(codec==QTextCodec::codecForName("BIG5"))
            {
                codec=QTextCodec::codecForName("BIG5");
                ui->actionBIG5->setChecked(true);
            }
            else
            {
                codec=QTextCodec::codecForName("GBK");
                ui->actionGBK->setChecked(true);
            }
            bomsize=0;
        }
    }
    filepos=settings->value(fn,0).toLongLong();
    realfilesize=file->size()-bomsize;
    if(filepos>=realfilesize)
    {
        filepos=0;
    }
    ui->verticalScrollBar->setMinimum(0);
    int maxscroll;
    if(realfilesize>2147483647)
    {
        maxscroll=2147483647;
    }
    else
    {
        maxscroll=realfilesize-1;
    }
    ui->verticalScrollBar->setMaximum(maxscroll);
    ui->verticalScrollBar->setEnabled(true);
    setScrollBarPosition();
    maxblock=realfilesize/POP_BLOCKSIZE;
    fileblock *fb=new fileblock(filepos/POP_BLOCKSIZE);
    stopcode=1;
    ui->widget->needrenew=true;
    ui->widget->update();
}

void MainWindow::wheelEvent(QWheelEvent *eve)
{
    QPoint t=eve->angleDelta();
    if(t.y()>0)
    {
        if(ui->widget->lineMove(true))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
            setScrollBarPosition();
        }
    }
    else
    {
        if(ui->widget->lineMove(false))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
            setScrollBarPosition();
        }
    }
}


void MainWindow::on_verticalScrollBar_actionTriggered(int action)
{
    switch(action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
        if(ui->widget->lineMove(false))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
        }
        setScrollBarPosition();
        break;
    case QAbstractSlider::SliderSingleStepSub:
        if(ui->widget->lineMove(true))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
        }
        setScrollBarPosition();
        break;
    case QAbstractSlider::SliderPageStepAdd:
        if(ui->widget->pageMove(false))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
        }
        setScrollBarPosition();
        break;
    case QAbstractSlider::SliderPageStepSub:
        if(ui->widget->pageMove(true))
        {
            ui->widget->needrenew=true;
            ui->widget->update();
        }
        setScrollBarPosition();
        break;
    default:
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *eve)
{
    switch(eve->key())
    {
    case Qt::Key_Up:
        ui->verticalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
        break;
    case Qt::Key_Down:
        ui->verticalScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        break;
    case Qt::Key_PageDown:
        ui->verticalScrollBar->triggerAction(QAbstractSlider::SliderPageStepAdd);
        break;
    case Qt::Key_PageUp:
        ui->verticalScrollBar->triggerAction(QAbstractSlider::SliderPageStepSub);
        break;
    default:
        QWidget::keyPressEvent(eve);
    }
}

void MainWindow::on_verticalScrollBar_sliderReleased()
{
    qint64 i;
    qreal r;
    i=ui->verticalScrollBar->value();
    if(realfilesize>2147483647)
    {
        r=i/2147483647;
        i=r*(realfilesize-1);
        if(i>=realfilesize)
        {
            i=realfilesize-1;
        }
    }
    if(ui->widget->randomMove(i))
    {
        ui->widget->needrenew=true;
        ui->widget->update();
    }
    setScrollBarPosition();
}

void MainWindow::setScrollBarPosition()
{
    if(realfilesize>2147483647)
    {
        int value;
        value=filepos/(realfilesize-1)*2147483647;
        ui->verticalScrollBar->setSliderPosition(value);
    }
    else
    {
        ui->verticalScrollBar->setSliderPosition(filepos);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    recentFile.removeDuplicates();
    int size=recentFile.size();
    if(size>10)
    {
        size-=10;
        for(int i=0;i<size;i++)
        {
            recentFile.removeLast();
        }
    }
    settings->setValue("_recentfiles",recentFile);
    if(file->isOpen())
    {
        settings->setValue(file->fileName(),filepos);
    }
}

/*void MainWindow::menuGBK()
{
    
}
void MainWindow::menuUnicode()
{
    
}
void MainWindow::menuUnicodeBE()
{
    
}
void MainWindow::menuUTF8()
{
    
}
void MainWindow::menuBIG5()
{
    
}*/

void MainWindow::menuCodec()
{
    QString ser=QObject::sender()->objectName();
    QList<QAction *> codecs=ui->menuCodec->actions();
    QAction* op;
    foreach (op, codecs) {
        op->setChecked(false);
    }
    
    if(ser=="actionGBK")
    {
        codec=QTextCodec::codecForName("GBK");
        ui->actionGBK->setChecked(true);
    }
    else if(ser=="actionUnicode")
    {
        codec=QTextCodec::codecForName("UTF-16LE");
        ui->actionUnicode->setChecked(true);
    }
    else if(ser=="actionUnicode_Big_Endian")
    {
        codec=QTextCodec::codecForName("UTF-16BE");
        ui->actionUnicode_Big_Endian->setChecked(true);
    }
    else if(ser=="actionUTF_8")
    {
        codec=QTextCodec::codecForName("UTF-8");
        ui->actionUTF_8->setChecked(true);
    }
    else if(ser=="actionBIG5")
    {
        codec=QTextCodec::codecForName("BIG5");
        ui->actionBIG5->setChecked(true);
    }
    else
        return;
    stopcode++;
    ui->widget->needrenew=true;
    ui->widget->update();
}

void MainWindow::menuAbout()
{
    if(!DAbout)
    {
        DAbout=new DialogAbout(this);
    }
    DAbout->exec();
}

void MainWindow::menuOptions()
{
    if(!DOptions)
    {
        DOptions=new DialogOptions(this);
    }
    DOptions->exec();
}
