#include "game.h"

DinoGame::DinoGame(){
    dino_temp = imread(QDir::currentPath().toStdString() + "/dino.jpg", cv::IMREAD_GRAYSCALE );
}

void DinoGame::LoadFrame(cv::Mat frame){
    cv::Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);
    cvtColor(frame, frame, CV_BGRA2BGR);
    ProcessFrame(gray, frame);
}


void DinoGame::ProcessFrame(cv::Mat& I, cv::Mat org_frame){
    CV_Assert(I.depth() == CV_8U);  //yalnizca tek kanalli/grayscale matrisler.
    long total_intensity = 0;
    int channels = I.channels();

    int H = I.rows;
    int W = I.cols * channels;
    int width = I.cols;
    int height = I.rows;

    if (I.isContinuous())
    {
        W *= H;
        H = 1;
    }

    int i,j;
    uchar* p;
    for( i = 0; i < H; ++i)
    {
        p = I.ptr<uchar>(i);
        for ( j = 0; j < W; ++j)
        {
            uchar val = p[j];
            if(val != 0){
                total_intensity+=val;
            }
        }
    }

    int avg_intensity = total_intensity / (width * height);
    this->is_night = (avg_intensity < 180) ? true : false;
    //std::cout << "avg_intensity: " << avg_intensity << std::endl;

    /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
    */

    cv::Mat t;
    int thresh_type = (this->is_night) ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
    threshold( I, t, 180, 255, thresh_type );

    //cv::erode(t,t, );
    //cv::dilate(t,t);

    int horizontalsize = width / 5;
    cv::Mat horizontalStructure = getStructuringElement(cv::MORPH_RECT, cv::Size(horizontalsize,1));
    cv::Mat ground_line = t.clone();

    erode(ground_line, ground_line, horizontalStructure, cv::Point(-1, -1));
    dilate(ground_line, ground_line, horizontalStructure, cv::Point(-1, -1));

    //imshow("horizontal", ground_line);
    /*std::vector<Vec2f> lines;
    cv::HoughLines(ground_line, lines, 1, CV_PI/180, 100, 0, 0);

    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( org_frame, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
    }*/

    std::vector<cv::Vec4i> linesP; // will hold the results of the detection
    HoughLinesP(ground_line, linesP, 1, CV_PI/180, 50, 50, 100 ); // runs the actual detection

    if(linesP.size() == 0){
        return;
    }

    int ly = -1;
    for( size_t i = 0; i < linesP.size(); i++ )
    {
        if(ly == -1){
            ly = linesP[i][1];
        }else{
            if(ly != linesP[i][1]){
                return;
            }
        }
        /*Vec4i l = linesP[i];
        line( org_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);*/
    }
    int line_y = ly;
    line( org_frame, cv::Point(0, line_y), cv::Point(width, line_y), cv::Scalar(255,0,0), 1, cv::LINE_AA);

    int dilation_size = 1;

    cv::Mat ed_element = getStructuringElement( cv::MORPH_RECT,
                                           cv::Size( 2*dilation_size  + 1, 2*dilation_size +1 ),
                                           cv::Point( dilation_size , dilation_size  ) );

    /*cv::erode( t, t, ed_element );
    cv::dilate( t, t, ed_element );*/

    //cv::blur( t, t, cv::Size( 3, 3 ), cv::Point(-1,-1) );

    if(!dino_found){
        dinoPos = findDino(t, line_y);
        collisionLine.top = cv::Point(dinoPos.right + 72, dinoPos.top);
        collisionLine.bottom = cv::Point(dinoPos.right + 72, dinoPos.top + (dinoPos.bottom - dinoPos.top)*0.8);
        dino_found = true;
    }



    imshow("tresh", t);
    bool collided = false;
    if(dino_found){
        cv::Mat dinoMat = org_frame.clone();
        rectangle(dinoMat, cv::Point(dinoPos.left, dinoPos.top), cv::Point(dinoPos.right, dinoPos.bottom), cv::Scalar(0, 0, 255));
        cv::Scalar color;
        collided = checkCollision(collisionLine, t);
        color = (collided) ? cv::Scalar(255,0,0) : cv::Scalar(0, 255, 0);
        line( dinoMat, collisionLine.top, collisionLine.bottom, color, 1, cv::LINE_AA);
        imshow("dino", dinoMat);
    }

    emit ProcessFinished(QPoint(), new QPoint(), this->is_night, org_frame, collided);
}

bool DinoGame::checkCollision(CollisionLine col, cv::Mat &I){
    CV_Assert(I.depth() == CV_8U);  //yalnizca tek kanalli/grayscale matrisler.

    int width = I.cols;
    int height = I.rows;


    int i,j;
    uchar* p;
    bool collided = false;
    for( i = col.top.y; i <= col.bottom.y; ++i)
    {
        p = I.ptr<uchar>(i);

        uchar val = p[col.top.x];
        if(val != 0){
            collided = true;
            break;
        }
    }
    return collided;
}


DinoPos DinoGame::findDino(cv::Mat &I, int ground_top){
    CV_Assert(I.depth() == CV_8U);  //yalnizca tek kanalli/grayscale matrisler.

    int width = I.cols;
    //int height = I.rows;

    int top = 10000;
    int right = 0;
    int left = 10000;
    int bottom = ground_top -3;
    bool l_ok = false;
    bool t_ok = false;
    bool b_ok = false;
    bool r_ok = false;
    b_ok = true;

    int i,j;
    uchar* p;
    for( i = ground_top-1; i >= 0; --i)
    {
        p = I.ptr<uchar>(i);
        bool left_found = false;
        for ( j = 0; j < width / 4; ++j)
        {
            uchar val = p[j];
            if(val != 0){
                if(!left_found){
                    if(j < left){
                        left = j;
                        left_found = true;
                        l_ok = true;
                    }
                    if(i < top){
                        top = i;
                        t_ok = true;
                    }
                }
            }else{
                if(left_found){
                    if(j > right){
                        right = j;
                        r_ok = true;
                    }
                    break;
                }
            }

            /*if(!left_found){
                break;
            }*/
        }
    }

    DinoPos pos;
    pos.left = left;
    pos.right = right;
    pos.top = top;
    pos.bottom = bottom;
    //std::cout << "l: " << left << "t: " << top << "b: " << bottom << "r: " << right << std::endl;
    pos.ok = ( l_ok && t_ok && b_ok && r_ok );
    /*if(pos.ok){
        std::cout << "Dino Bulundu: " << std::endl;
    }*/
    return pos;

}

