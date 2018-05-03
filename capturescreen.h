#ifndef CAPTURESCREEN_H
#define CAPTURESCREEN_H

#include <QWidget>
#include <QPainter>
#include <QList>


typedef struct _SCREEN_MAP
{
    int     iScreenIndex;   //屏幕的索引
    QRect   rect;           //屏幕的区域
    QPixmap pixmap;         //屏幕截图
}SCREEN_MAP;




class CaptureScreen : public QWidget
{
    Q_OBJECT

public:
    CaptureScreen(QWidget *parent = 0);
    ~CaptureScreen();

Q_SIGNALS:
    void signalCompleteCature(QPixmap catureImage);

public:
    void initWindow();
    void loadBackgroundPixmap();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint);

public:
    QList<SCREEN_MAP> m_listScreenMap; //多屏

private:
    bool m_isMousePress;
    QPixmap m_loadPixmap, m_capturePixmap;
    int m_screenwidth;
    int m_screenheight;
    QPoint m_beginPoint, m_endPoint;
    QPainter m_painter;
};

#endif // CAPTURESCREEN_H
