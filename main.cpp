#include "qocr.h"
#include <QApplication>

#include "logindialog.h"  //账号登录对话框


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //QOcr w;
    //w.show();
    LoginDialog loginDialog;
    loginDialog.show();

    return a.exec();
}
