#include "captureworker.h"

CaptureWorker::CaptureWorker(QDesktopWidget *d, QScreen *sc){
    this->desktop = d;
    this->screen = sc;
    is_working = false;
    std::cout << "cw init! \n";
}


void CaptureWorker::startWorkerThread(){
    is_working = true;
    while(is_working){
        UpdateFrame();
        QThread::sleep(16);
    }
}

void CaptureWorker::stopWorkerThread(){
    is_working = false;
}

void CaptureWorker::UpdateFrame(){
    //std::cout << "frame!\n";
    QRect crop_rect(0, 0, 1366, 768);

    QPixmap pix = screen->grabWindow(
        desktop->winId(),
        crop_rect.left(),
        crop_rect.top(),
        crop_rect.width(),
        crop_rect.height());

    /*pix = pix.scaled(imgView->size(),Qt::KeepAspectRatio);
            imgView->setPixmap(pix);*/
    std::cout << "frame! \n";
    emit frameCreated(pix);
}

void CaptureWorker::SetBounds(int x, int y, int w, int h){
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}


CaptureWorker::~CaptureWorker(){
    delete screen;
}
