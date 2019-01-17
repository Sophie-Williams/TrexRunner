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
#include <QtMath>


//using namespace cv;

struct DinoPos{
    int top;
    int left;
    int right;
    int bottom;
    bool ok;
};

struct CollisionResult{
    bool top_collision = false;
    bool bottom_collision = false;
    bool dino_collision = false;
    bool safe = false;
};

/*struct CollisionLine{
    cv::Point top;
    cv::Point bottom;
};*/

class DinoGame : public QObject{
    Q_OBJECT

public:
    void LoadFrame(cv::Mat frame);
    DinoGame();

private:
    bool game_initialized = false;
    bool is_night = false;
    bool ground_found = false;
    bool on_ground = false;
    int ground_y = -1;
    bool dino_found = false;
    void ProcessFrame(cv::Mat &I, cv::Mat org_frame);
    cv::Mat dino_temp, dino_temp2;
    cv::Mat thresh_mat;
    cv::Rect dino_pos;
    cv::Rect first_dino_pos;
    cv::Rect collision_area;
    cv::Rect findDinoItr(cv::Mat& I, int ground_top);
    cv::Rect findDinoTM( cv::Mat& I, cv::Mat templ );
    bool detectCollision(cv::Mat &I, cv::Rect roi, bool ignore_dino = false);
    CollisionResult coll_result;
    int findGround(cv::Mat& I);


signals:
    void ProcessFinished(QPoint dino, QPoint blocks[], bool is_night, cv::Mat frame, CollisionResult coll_data, bool on_ground);

};

#endif // GAME_H
