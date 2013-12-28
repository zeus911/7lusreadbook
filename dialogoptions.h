//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>
#include "textout.h"

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogOptions(QWidget *parent = 0);
    ~DialogOptions();
    
private slots:
    void on_pushButtonClose_clicked();
    
    void on_pushButtonFont_clicked();
    
    void on_pushButton_clicked();
    
    void on_pushButtonFColor_clicked();
    
    void on_pushButtonBColor_clicked();
    
    void on_pushButtonReset_clicked();
    
private:
    Ui::DialogOptions *ui;
    QFont savedfont,tmpfont;
    QColor frontcolor,tfcolor;
    QColor bgcolor,tbcolor;
    qreal linespace,padding;
    int maxblocksize;
    void showEvent(QShowEvent *);
};

#endif // DIALOGOPTIONS_H
