#ifndef QOCR_H
#define QOCR_H

#include <QWidget>
#include <QDebug>

namespace Ui {
class QOcr;
}

class QOcr : public QWidget
{
    Q_OBJECT

public:
    explicit QOcr(QWidget *parent = 0);
    ~QOcr();

public slots:
    //接受 账号登录对话框发送来的信号(携带了3个参数)
    void RecvSignalArgs(bool bUseDefaultAccount, QString strAppId, QString strApiKey, QString strSecretKey);

    //接受 账号登录对话框 发来的 "取消"信号, 退出程序
    void RecvSingalExit();

    void on_pushButton_clicked();

    //在label上显示截取的图片
    void onCompleteCature(QPixmap captureImage);

    //保存图片
    //void SaveImage(QPixmap captureImage);

Q_SIGNALS:
    //成功保存图片
     void signalSaveImageComplete();
     //void signalStarOCR(); //开始识别


public:
    //测试libcurl
    void TestCurl();

public:
    //测试 jsoncpp
    void TestJsonCpp();

public slots:
    //测试图像识别 api
    void TestAIP();

public:

    //测试 OpenSSL 库的 AES加密接口
    void TestAES();

    //加密
    void Encode(char *plainStr, unsigned char **cryper);

    //解密
    void Decode(unsigned char **cryper, char *plainStr);


private:
    QString m_strAppId;
    QString m_strApiKey;
    QString m_strSecretKey;

 //   LoginDialog *m_LoginDlg;
    std::string m_strImgPath;

    bool m_bUseDefaultAccount; //使用默认账号


private:
    Ui::QOcr *ui;
};

#endif // QOCR_H
