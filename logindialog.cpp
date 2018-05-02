#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QCryptographicHash>

#include <QPixmap>
#include <QDateTime>
#include <QScreen>
#include <QDesktopWidget>


LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);


    m_QOcr = new QOcr();

    //显示主程序界面
    QObject::connect(this, SIGNAL(sendShowMainWidget()), m_QOcr, SLOT(show()) );

    //将账号信息发送给主界面
    QObject::connect(this, SIGNAL(sendStrings(QString,QString,QString)), m_QOcr, SLOT(RecvSignalArgs(QString,QString,QString)) );
}

LoginDialog::~LoginDialog()
{
    delete ui;
    if(m_QOcr)
    {
        delete m_QOcr;
        m_QOcr = nullptr;
    }
}

bool LoginDialog::WriteInfo(QString strAppId, QString strApiKey, QString strSecretKey)
{

}

void LoginDialog::on_btnCancel_clicked()
{
    this->close();
}

void LoginDialog::on_btnOk_clicked()
{

    //发送一个信号, 把账号登录界面 输入的三个值
    //appid, apikey, secret eky 发送给主界面
    QString strAppID = ui->lEditAppID->text().trimmed();
    QString strApiKey = ui->lineEditAPIKey->text().trimmed();
    QString strSecretKey = ui->lineEditSecretKey->text().trimmed();

    if(strAppID.isEmpty())
    {
        QMessageBox::warning(this, "提示", "appId为空, 请重新输入");
        return;
    }
    if(strApiKey.isEmpty())
    {
        QMessageBox::warning(this, "提示", "apiKey为空, 请重新输入");
        return;
    }
    if(strSecretKey.isEmpty())
    {
        QMessageBox::warning(this, "提示", "secrete key为空, 请重新输入");
        return;
    }

    //如果选择了 "记住信息" 应该讲账户写入文件



    emit sendShowMainWidget();

    emit sendStrings(strAppID, strApiKey, strSecretKey);



    this->hide();  //隐藏登录界面

}
