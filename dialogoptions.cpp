//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#include "dialogoptions.h"
#include "ui_dialogoptions.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QSettings>
extern MainWindow *w;
extern unsigned int stopcode;
extern QSettings *settings;

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
    maxblocksize=w->ui->widget->maxblockinmemory;
    savedfont=*(w->ui->widget->font);
    frontcolor=w->ui->widget->fontcolor->color();
    bgcolor=w->ui->widget->background->color();
    linespace=w->ui->widget->linespace;
    padding=w->ui->widget->padding;
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

void DialogOptions::on_pushButtonClose_clicked()
{
    this->close();
}

void DialogOptions::on_pushButtonFont_clicked()
{
    bool ok;
    QFont tf=QFontDialog::getFont(&ok,tmpfont,this);
    if(ok)
    {
        tmpfont=tf;
        //tmpfont.setLetterSpacing(QFont::AbsoluteSpacing,1.0);
    }
}

void DialogOptions::on_pushButton_clicked()
{
    bool updatefont=false;
    bool updatetext=false;
    if(savedfont!=tmpfont)
    {
        *(w->ui->widget->font)=tmpfont;
        //w->ui->widget->font->setLetterSpacing(QFont::AbsoluteSpacing,1.0);
        w->ui->widget->renewFontInfo();
        updatefont=true;
        updatetext=true;
        settings->setValue("_fontfamily",tmpfont.family());
        settings->setValue("_fontsize",tmpfont.pointSize());
        settings->setValue("_fontweight",tmpfont.weight());
        settings->setValue("_fontitalic",tmpfont.italic());
    }
    if(padding!=ui->doubleSpinBoxPadding->value())
    {
        w->ui->widget->padding=padding=ui->doubleSpinBoxPadding->value();
        updatetext=true;
        updatefont=true;
        settings->setValue("_padding",padding);
    }
    if(linespace!=ui->doubleSpinBoxLineSpace->value())
    {
        w->ui->widget->linespace=linespace=ui->doubleSpinBoxLineSpace->value();
        updatetext=true;
        settings->setValue("_linespace",linespace);
    }
    if(maxblocksize!=ui->spinBoxMaxBlocks->value())
    {
        w->ui->widget->maxblockinmemory=maxblocksize=ui->spinBoxMaxBlocks->value();
        settings->setValue("_maxblockinmemory",maxblocksize);
    }
    if(frontcolor!=tfcolor)
    {
        frontcolor=tfcolor;
        w->ui->widget->fontcolor->setColor(frontcolor);
        updatetext=true;
        settings->setValue("_fontr",tfcolor.red());
        settings->setValue("_fontg",tfcolor.green());
        settings->setValue("_fontb",tfcolor.blue());
    }
    if(bgcolor!=tbcolor)
    {
        bgcolor=tbcolor;
        w->ui->widget->background->setColor(bgcolor);
        updatetext=true;
        settings->setValue("_bgr",tbcolor.red());
        settings->setValue("_bgg",tbcolor.green());
        settings->setValue("_bgb",tbcolor.blue());
    }
    if(updatefont)
    {
        savedfont=*(w->ui->widget->font);
        stopcode++;
    }
    if(updatetext)
    {
        w->ui->widget->needrenew=true;
        w->ui->widget->update();
    }
    this->close();
}

void DialogOptions::showEvent(QShowEvent *)
{
    tmpfont=savedfont;
    tfcolor=frontcolor;
    tbcolor=bgcolor;
    ui->spinBoxMaxBlocks->setValue(maxblocksize);
    ui->doubleSpinBoxLineSpace->setValue(linespace);
    ui->doubleSpinBoxPadding->setValue(padding);
}

void DialogOptions::on_pushButtonFColor_clicked()
{
    QColor tc=QColorDialog::getColor(tfcolor,this,tr("前景色"));
    if(tc.isValid())
    {
        tfcolor=tc;
    }
}

void DialogOptions::on_pushButtonBColor_clicked()
{
    QColor tc=QColorDialog::getColor(tbcolor,this,tr("背景色"));
    if(tc.isValid())
    {
        tbcolor=tc;
    }
}

void DialogOptions::on_pushButtonReset_clicked()
{
    ui->spinBoxMaxBlocks->setValue(100);
    ui->doubleSpinBoxLineSpace->setValue(8.0);
    ui->doubleSpinBoxPadding->setValue(1.0);
    tmpfont.setFamily("Sans");
    tmpfont.setPointSize(30);
    tmpfont.setWeight(QFont::Normal);
    tmpfont.setItalic(false);
    //tmpfont.setLetterSpacing(QFont::AbsoluteSpacing,1.0);
    tfcolor.setRgb(127,255,255);
    tbcolor.setRgb(0,0,0);
}
