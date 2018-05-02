#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>



#include "qocr.h"


namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();


public:

    //写文件
    bool WriteInfo(QString strAppId, QString strApiKey, QString strSecretKey);



private slots:
    void on_btnCancel_clicked();

    void on_btnOk_clicked();



signals:
    //发送账号信息
    void sendStrings(QString strAppId, QString strApiKey, QString strSecretKey);

    //显示主界面
    void sendShowMainWidget();


private:

    QOcr *m_QOcr;  //把操作界面, 做为子界面



private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
