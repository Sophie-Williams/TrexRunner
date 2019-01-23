#include "game.h"

DinoGame::DinoGame(){
    dino_temp = imread(QDir::currentPath().toStdString() + "/dino.jpg", cv::IMREAD_GRAYSCALE );
    dino_temp2 = imread(QDir::currentPath().toStdString() + "/dino2.jpg", cv::IMREAD_GRAYSCALE );
    start_speed = 320;
    game_speed = start_speed;
    contour_timer = new QTimer();
    contour_timer->setInterval(1000);
    connect(contour_timer, SIGNAL(timeout()), this, SLOT(InspectContours()));
    //contour_timer->start();

    safe_zone = cv::Rect(-1,-1,-1,-1);
    dino_pos = cv::Rect(-1,-1,-1,-1);
    first_dino_pos = cv::Rect(-1,-1,-1,-1);
    collision_area = cv::Rect(-1,-1,-1,-1);

    game_timer = new QTimer();
    game_timer->setInterval(1000);
    connect(game_timer, SIGNAL(timeout()), this, SLOT(GameTimeTick()));
    //game_timer->start();
}

DinoGame::~DinoGame(){
    delete contour_timer;
    delete game_timer;
}

void DinoGame::GameTimeTick(){
    total_growth += growth_factor * total_growth;
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

    current_bounding_rects.clear();

    for (size_t i = 0; i < contours.size(); i++) {

        cv::Rect bounding_rect = cv::boundingRect(contours[i]);
        if(bounding_rect.height < 20){
            continue;
        }
        //cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
        current_bounding_rects.push_back(bounding_rect);
        cv::rectangle( result_frame, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(0,0,255), 1, 8, 0 );
    }

    //cv::imshow("contours", contourImage);




    /* end */





    if(dino_found){
        //dino_pos = findDinoTM(I, dino_temp);
    }else{
        first_dino_pos = findDinoTM(I, dino_temp);
        dino_pos = first_dino_pos;
        dino_found = true;
    }



    rectangle(result_frame,
              first_dino_pos.tl(),
              first_dino_pos.br(),
              cv::Scalar(0, 255, 0));

    /*rectangle(result_frame,
              dino_pos.tl(),
              dino_pos.br(),
              cv::Scalar(0, 0, 255));*/



    if(game_timer->isActive()){
        int coll_right = first_dino_pos.x + first_dino_pos.width + 4 + collision_area.width + 10;
        if(coll_right >= I.cols - 30){
            game_timer->stop();
        }
    }


    int speed_x_1 = I.cols * 0.95;
    int speed_x_2 = I.cols * 0.60;

    cv::Rect speed_collider_1(
                speed_x_1,
                I.rows / 3,
                1,
                I.rows - ((I.rows  - ground_y) + (I.rows / 3)) - 5
                );

    cv::Rect speed_collider_2(
                speed_x_2,
                I.rows / 3,
                1,
                I.rows - ((I.rows  - ground_y) + (I.rows / 3)) - 5
                );



    if(speed_coll_1 == false){
        speed_coll_1 = detectCollision(thresh_mat, speed_collider_1);
        speed_timer.restart();
    }else{
        if(speed_coll_1 == true && speed_coll_2 == false){
            speed_coll_2 = detectCollision(thresh_mat, speed_collider_2);
        }

        if(speed_coll_1 == true && speed_coll_2 == true){
            long long t = speed_timer.elapsed();
            if(t != 0){
                int s = (speed_x_1 - speed_x_2) * 1000 / t;
                speed_values.push_back(s);
                if(speed_values.size() == 5){
                    int total_s = 0;
                    for(int i=0; i<speed_values.size(); i++){
                        total_s += speed_values[i];
                    }
                    game_speed = total_s / 5;
                    speed_values.clear();
                }
            }

        }
    }


    collision_area = cv::Rect(
                first_dino_pos.x + first_dino_pos.width + 4,
                first_dino_pos.y,
                first_dino_pos.width * 1.3 + (game_speed - start_speed)/4,
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


    safe_zone = cv::Rect(
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
    coll_result.safe = !detectCollision(thresh_mat, safe_zone, true);

    rectangle(result_frame,
              topCollBox,
              cv::Scalar(255, 0, 0));

    cv::Scalar speed_color_1 = (speed_coll_1) ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 0, 0);
    cv::Scalar speed_color_2 = (speed_coll_2) ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 0, 0);

    rectangle(result_frame,
              speed_collider_1,
              speed_color_1);

    rectangle(result_frame,
              speed_collider_2,
              speed_color_2);

    if(speed_coll_1 && speed_coll_2){
        speed_coll_1 = speed_coll_2 = false;
    }

    rectangle(result_frame,
              bottomCollBox,
              cv::Scalar(255, 0, 0));

    rectangle(result_frame,
              safe_zone,
              cv::Scalar(0, 255, 255));

    //bool on_ground = (dino_pos.y == first_dino_pos.y ) ? true : false;


    emit ProcessFinished(QPoint(), new QPoint(), this->is_night, result_frame, coll_result, on_ground, game_speed);
}


void DinoGame::InspectContours(){
    if(current_bounding_rects.size() == 0){
        return;
    }

    if(current_bounding_rects.size() > 0 && previous_bounding_rects.size() == 0){
        for(int i=0; i<current_bounding_rects.size(); i++){
            previous_bounding_rects.push_back(current_bounding_rects[i]);
        }
        return;
    }

    if(current_bounding_rects.size() == previous_bounding_rects.size()){
        bool no_movement = true;
        for(int i=0; i<current_bounding_rects.size(); i++){
            if(current_bounding_rects[i].x != previous_bounding_rects[i].x){
                no_movement = false;
                break;
            }
        }

        if(no_movement){
            emit GameOver();
            return;
        }

        for(int i=0; i<current_bounding_rects.size(); i++){
            if(
                current_bounding_rects[i].x == previous_bounding_rects[i].x &&
                current_bounding_rects[i].width == previous_bounding_rects[i].width &&
                current_bounding_rects[i].height == previous_bounding_rects[i].height
            ){
                if(current_bounding_rects[i].y == previous_bounding_rects[i].y){
                    on_ground = true;
                }else if(current_bounding_rects[i].y != previous_bounding_rects[i].y){
                    on_ground = false;
                }
                break;
            }
        }

    }


    previous_bounding_rects.clear();
    for(int i=0; i<current_bounding_rects.size(); i++){
        previous_bounding_rects.push_back(current_bounding_rects[i]);
    }


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

