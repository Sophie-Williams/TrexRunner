#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QScreen>
#include <QLabel>
#include <QDesktopWidget>
#include <iostream>
#include <QPixmap>
#include <QTimer>
#include "area_selector.h"
#include <QTextEdit>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QKeyEvent>
#include <QEvent>
#include "controller.h"

#include "game.h"

#include "Windows.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLabel *imgCaptured;
    QLabel *imgInfo;
    QTextEdit *txtInfo;
    QTimer *timer;
    QTimer *fps_timer;
    QTimer *key_release_timer;
    QRect bounds;
    DinoGame *game;

public slots:
    void Tick();
    void GetFPS();
    void Start(QRect rect);
    void Update(QPoint dino, QPoint blocks[], bool is_night, cv::Mat frame, CollisionResult coll_data, bool on_ground);
    void ReleaseKey();

private:
    Ui::MainWindow *ui;
    BoundSelector *bound_selector;
    int frame_count = 0;
    cv::Mat qimage_to_mat_ref(QImage const &img, int format);
    QPixmap mat_to_qimage_cpy(cv::Mat const &mat, QImage::Format format);
    QPixmap pix;
    int current_fps;
    bool collided = false;
    void ShowImage(QPixmap pix, QLabel *canvas);
    void SendKey(WORD key);
    WORD last_key;
    bool on_down_press = false;
    bool on_up_press = false;
    WORD UP_KEY = 0x26;
    WORD DOWN_KEY = 0x28;
    GameController controller;
};

#endif // MAINWINDOW_H
