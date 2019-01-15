#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QPoint>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include <vector>
#include <QDir>


//using namespace cv;

struct DinoPos{
    int top;
    int left;
    int right;
    int bottom;
    bool ok;
};

struct CollisionLine{
    cv::Point top;
    cv::Point bottom;
};

class DinoGame : public QObject{
    Q_OBJECT

public:
    void LoadFrame(cv::Mat frame);
    DinoGame();

private:
    bool game_initialized = false;
    bool is_night = false;
    bool ground_found = false;
    void ProcessFrame(cv::Mat &I, cv::Mat org_frame);
    cv::Mat dino_temp;
    DinoPos dinoPos;
    DinoPos findDino(cv::Mat& I, int ground_top);
    bool dino_found = false;
    CollisionLine collisionLine;
    bool checkCollision(CollisionLine col, cv::Mat& I);



signals:
    void ProcessFinished(QPoint dino, QPoint blocks[], bool is_night, cv::Mat frame, bool collided);

};

#endif // GAME_H
