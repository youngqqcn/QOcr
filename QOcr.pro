#-------------------------------------------------
#
# Project created by QtCreator 2018-04-30T10:17:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QOcr
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        qocr.cpp \
    logindialog.cpp \
    capturescreen.cpp

HEADERS += \
        qocr.h \
    logindialog.h \
    capturescreen.h

FORMS += \
        qocr.ui \
    logindialog.ui

CONFIG += c++11


#LIBS += -L"C:/OpenSSL-Win32/bin" -llibeay32
#LIBS += -L"C:/OpenSSL-Win32/bin" -lssleay32
#LIBS += -L"C:/curl-7.59.0/builds/libcurl-vc10-x86-release-dll-ssl-static/lib" -llibcurl
#INCLUDEPATH += $$quote(C:/OpenSSL-Win32/include)
#INCLUDEPATH += $$quote(C:/curl-7.59.0/builds/libcurl-vc10-x86-release-dll-ssl-static/include)
#INCLUDEPATH += $$quote(C:/Users/yqq/Documents/QtProjects/QOcr/aip-cpp-sdk-0.6.0)

LIBS += -L"C:/Users/yqq/Documents/QtProjects/QOcr/OpenSSL/bin" -llibeay32
LIBS += -L"C:/Users/yqq/Documents/QtProjects/QOcr/OpenSSL/bin" -lssleay32
LIBS += -L"C:/Users/yqq/Documents/QtProjects/QOcr/libcurl/lib" -llibcurl
INCLUDEPATH += $$quote(C:/Users/yqq/Documents/QtProjects/QOcr/OpenSSL/include)
INCLUDEPATH += $$quote(C:/Users/yqq/Documents/QtProjects/QOcr/libcurl/include)
INCLUDEPATH += $$quote(C:/Users/yqq/Documents/QtProjects/QOcr/aip-cpp-sdk-0.6.0)



include(json/json.pri)

















