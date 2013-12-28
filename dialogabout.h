//Copyright (C) 2013  黄思亿
//请查看LICENSE.GPL文件以获得完整的授权协议，联系作者请发送邮件到popkcer@gmail.com
#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>

namespace Ui {
class DialogAbout;
}

class DialogAbout : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogAbout(QWidget *parent = 0);
    ~DialogAbout();
    
private slots:
    void on_pushButton_clicked();
    
private:
    Ui::DialogAbout *ui;
};

#endif // DIALOGABOUT_H
