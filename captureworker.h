#ifndef CAPTUREWORKER_H
#define CAPTUREWORKER_H

#include <QObject>
#include <QMutex>
#include <QRect>
#include <QPixmap>
#include <QDesktopWidget>
#include <QScreen>
#include <iostream>
#include "atomic"
#include <QThread>
#include <QImage>

class CaptureWorker: public QObject{
    Q_OBJECT
private:
    QMutex worker_mutex;
    std::atomic<bool> is_working;
    int x, y, w, h;
    QScreen *screen;
    QDesktopWidget *desktop;
public:
    CaptureWorker(QDesktopWidget *d, QScreen *sc);
    ~CaptureWorker();
    void SetBounds(int x, int y, int w, int h);
    void UpdateFrame();

public slots:
    void stopWorkerThread();
    void startWorkerThread();


signals:
    void frameCreated(QPixmap pix);
};


#endif // CAPTUREWORKER_H
