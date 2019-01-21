#include "game.h"

DinoGame::DinoGame(){
    dino_temp = imread(QDir::currentPath().toStdString() + "/dino.jpg", cv::IMREAD_GRAYSCALE );
    dino_temp2 = imread(QDir::currentPath().toStdString() + "/dino2.jpg", cv::IMREAD_GRAYSCALE );
}

void DinoGame::LoadFrame(cv::Mat frame){
    cv::Mat gray;
    cvtColor(frame, gray, CV_BGRA2GRAY);
    cvtColor(frame, frame, CV_BGRA2BGR);
    ProcessFrame(gray, frame);
}


void DinoGame::ProcessFrame(cv::Mat& I, cv::Mat org_frame){
    cv::Mat result_frame = org_frame.clone();
    if(!ground_found){
        ground_y = findGround(I);
        if(ground_y == -1){
            return;
        }
    }

    line( result_frame, cv::Point(0, ground_y), cv::Point(I.cols, ground_y), cv::Scalar(255,255,0), 1, cv::LINE_AA);



    /* find contours */

    cv::Mat basic_frame;

    int horizontalsize = I.cols / 5;
    int kernel_size = 1;
    cv::Mat horizontalStructure = getStructuringElement(cv::MORPH_ELLIPSE,
                                                        cv::Size( 2*kernel_size + 1, 2*kernel_size+1 ),
                                                        cv::Point( kernel_size, kernel_size ) );

    erode(thresh_mat, thresh_mat, horizontalStructure);

    horizontalStructure = getStructuringElement(cv::MORPH_RECT,
                                                        cv::Size( 6*kernel_size + 1, 6*kernel_size+1 ),
                                                        cv::Point( kernel_size*3, kernel_size*3 ) );

    dilate(thresh_mat, thresh_mat, horizontalStructure);
    //blur( thresh_mat, thresh_mat, cv::Size( kernel_size, kernel_size ), cv::Point(-1,-1) );


    //Find the contours. Use the contourOutput Mat so the original image doesn't get overwritten
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat contourOutput = thresh_mat.clone();
    cv::findContours( contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE );

    //Draw the contours
    cv::Mat contourImage(thresh_mat.size(), CV_8UC3, cv::Scalar(0,0,0));
    cv::Scalar colors[3];
    colors[0] = cv::Scalar(255, 0, 0);
    colors[1] = cv::Scalar(0, 255, 0);
    colors[2] = cv::Scalar(0, 0, 255);
    for (size_t idx = 0; idx < contours.size(); idx++) {
        cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
        cv::Rect bounding_rect = cv::boundingRect(contours[idx]);
        cv::rectangle( contourImage, bounding_rect.tl(), bounding_rect.br(), colors[idx % 3], 2, 8, 0 );
    }

    cv::imshow("contours", contourImage);




    /* end */





    if(dino_found){
        dino_pos = findDinoTM(I, dino_temp);
    }else{
        first_dino_pos = findDinoTM(I, dino_temp);
        dino_pos = first_dino_pos;
        dino_found = true;
    }



    rectangle(result_frame,
              first_dino_pos.tl(),
              first_dino_pos.br(),
              cv::Scalar(0, 255, 0));

    rectangle(result_frame,
              dino_pos.tl(),
              dino_pos.br(),
              cv::Scalar(0, 0, 255));

    collision_area = cv::Rect(
                first_dino_pos.x + first_dino_pos.width + 4,
                first_dino_pos.y,
                first_dino_pos.width * 1.2,
                first_dino_pos.height * 0.7
                );

    cv::Rect topCollBox(
                collision_area.x + (first_dino_pos.height - first_dino_pos.width)*3 + 5,
                collision_area.y,
                collision_area.width,
                collision_area.height/2
                );

    cv::Rect bottomCollBox(
                collision_area.x + (first_dino_pos.height - first_dino_pos.width)*3 + 5,
                collision_area.y + collision_area.height/2 + 5,
                collision_area.width + 10,
                collision_area.height/2 - 5
                );

    cv::Rect dangerZone(
                first_dino_pos.x,
                first_dino_pos.y - first_dino_pos.height,
                bottomCollBox.x + bottomCollBox.width - first_dino_pos.x,
                bottomCollBox.y + bottomCollBox.height - first_dino_pos.y + first_dino_pos.height
                );

    /*rectangle(result_frame,
              collision_area,
              cv::Scalar(255, 0, 0));*/


    coll_result.top_collision = detectCollision(thresh_mat, topCollBox);
    coll_result.bottom_collision = detectCollision(thresh_mat, bottomCollBox);
    coll_result.dino_collision = detectCollision(thresh_mat, first_dino_pos);
    coll_result.safe = !detectCollision(thresh_mat, dangerZone, true);

    rectangle(result_frame,
              topCollBox,
              cv::Scalar(255, 0, 0));

    rectangle(result_frame,
              bottomCollBox,
              cv::Scalar(255, 0, 0));

    rectangle(result_frame,
              dangerZone,
              cv::Scalar(0, 255, 255));

    bool on_ground = (dino_pos.y == first_dino_pos.y ) ? true : false;

    emit ProcessFinished(QPoint(), new QPoint(), this->is_night, result_frame, coll_result, on_ground);
}



int DinoGame::findGround(cv::Mat &I){
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

    /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
    */

    //cv::Mat t;
    int thresh_type = (this->is_night) ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
    threshold( I, thresh_mat, 180, 255, thresh_type );

    int horizontalsize = width / 5;
    cv::Mat horizontalStructure = getStructuringElement(cv::MORPH_RECT, cv::Size(horizontalsize,1));
    cv::Mat ground_line = thresh_mat.clone();

    erode(ground_line, ground_line, horizontalStructure, cv::Point(-1, -1));
    dilate(ground_line, ground_line, horizontalStructure, cv::Point(-1, -1));

    std::vector<cv::Vec4i> linesP;
    HoughLinesP(ground_line, linesP, 1, CV_PI/180, 50, 50, 100 );

    if(linesP.size() == 0){
        return -1;
    }

    int ly = -1;
    for( size_t i = 0; i < linesP.size(); i++ )
    {
        if(ly == -1){
            ly = linesP[i][1];
        }else{
            if(ly != linesP[i][1]){
                return -1;
            }
        }
    }
    int line_y = ly;
    return line_y;
}

bool DinoGame::detectCollision(cv::Mat& I, cv::Rect roi, bool ignore_dino){
    CV_Assert(I.depth() == CV_8U);  //yalnizca tek kanalli/grayscale matrisler.

    int right = roi.x + roi.width;
    int bottom = roi.y + roi.height;

    int i,j;
    uchar* p;
    for( i = roi.y; i <= bottom; ++i)
    {
        p = I.ptr<uchar>(i);
        for(j=roi.x; j<right; ++j){
            uchar val = p[j];
            if(val > 0){
                if(ignore_dino){
                    if(j >= dino_pos.x && j <= dino_pos.x + dino_pos.width && i >= dino_pos.y && i <= dino_pos.y + dino_pos.height){
                        continue;
                    }
                }
                return true;
            }
        }
    }
    return false;
}


cv::Rect DinoGame::findDinoItr(cv::Mat& I, int ground_top){
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

        }
    }

    cv::Rect pos;
    pos.x = left;
    pos.width = right - left;
    pos.y = top;
    pos.height = bottom - top;
    return pos;

}



cv::Rect DinoGame::findDinoTM(cv::Mat &I, cv::Mat templ ){
  cv::Mat img_display, result;
  I.copyTo( img_display );

  cv::Rect ROI(0, 0, I.cols/2, I.rows);

  I = I(ROI);

  int match_method = cv::TM_CCOEFF_NORMED;

  int result_cols =  I.cols - templ.cols + 1;
  int result_rows = I.rows - templ.rows + 1;

  result.create( result_rows, result_cols, CV_32FC1 );

  matchTemplate( I, templ, result, match_method );
  normalize( result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

  double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
  cv::Point matchLoc;

  minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

  if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
  else
    { matchLoc = maxLoc; }

  rectangle( img_display, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
  rectangle( result, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );

  cv::Rect pos;
  pos.x = matchLoc.x;
  pos.y = matchLoc.y;
  pos.width = templ.cols;
  pos.height =  templ.rows;
  return pos;
}

