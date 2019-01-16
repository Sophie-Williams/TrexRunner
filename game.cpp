#include "game.h"

DinoGame::DinoGame(){
    dino_temp = imread(QDir::currentPath().toStdString() + "/dino.jpg", cv::IMREAD_GRAYSCALE );
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

    /*int dilation_size = 1;

    cv::Mat ed_element = getStructuringElement( cv::MORPH_RECT,
                                           cv::Size( 2*dilation_size  + 1, 2*dilation_size +1 ),
                                           cv::Point( dilation_size , dilation_size  ) );*/

    /*cv::erode( t, t, ed_element );
    cv::dilate( t, t, ed_element );*/

    //cv::blur( t, t, cv::Size( 3, 3 ), cv::Point(-1,-1) );

    if(dino_found){
        dino_pos = findDinoTM(I, dino_temp);
    }else{
        //dinoPos = findDinoItr(t, line_y);
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
                first_dino_pos.width * 1.1,
                first_dino_pos.height * 0.7
                );

    rectangle(result_frame,
              collision_area,
              cv::Scalar(255, 0, 0));


    collided = detectCollision(thresh_mat, collision_area);

    //cv::imshow("img", result_frame);

    //line( org_frame, collision_line.top, collision_line.bottom, color, 1, cv::LINE_AA);

    //cv::imshow("dino_temp", dino_temp);
    //MatchingMethod(I, dino_temp);

    //imshow("tresh", t);
    //imshow("gray", I);
    /*bool collided = false;
    if(dino_found){
        cv::Mat dinoMat = org_frame.clone();

        cv::Scalar color;
        collided = checkCollision(collision_line, t);
        color = (collided) ? cv::Scalar(255,0,0) : cv::Scalar(0, 255, 0);
        line( dinoMat, collision_line.top, collision_line.bottom, color, 1, cv::LINE_AA);
        imshow("dino", dinoMat);
    }*/

    emit ProcessFinished(QPoint(), new QPoint(), this->is_night, result_frame, collided);
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
    //std::cout << "avg_intensity: " << avg_intensity << std::endl;

    /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
    */

    //cv::Mat t;
    int thresh_type = (this->is_night) ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
    threshold( I, thresh_mat, 180, 255, thresh_type );

    //cv::erode(t,t, );
    //cv::dilate(t,t);

    int horizontalsize = width / 5;
    cv::Mat horizontalStructure = getStructuringElement(cv::MORPH_RECT, cv::Size(horizontalsize,1));
    cv::Mat ground_line = thresh_mat.clone();

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
        /*Vec4i l = linesP[i];
        line( org_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);*/
    }
    int line_y = ly;
    return line_y;
    //line( org_frame, cv::Point(0, line_y), cv::Point(width, line_y), cv::Scalar(255,0,0), 1, cv::LINE_AA);
}

bool DinoGame::detectCollision(cv::Mat& I, cv::Rect roi){
    CV_Assert(I.depth() == CV_8U);  //yalnizca tek kanalli/grayscale matrisler.
    //cv::Mat col_area = I(roi);
    //int width = I.cols;
    //int height = I.rows;

    int right = roi.x + roi.width;
    int bottom = roi.y + roi.height;

    int i,j;
    uchar* p;
    for( i = roi.y; i <= bottom; ++i)
    {
        p = I.ptr<uchar>(i);
        for(int j=roi.x; j<right; ++j){
            uchar val = p[j];
            if(val > 0){
                return true;;
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

            /*if(!left_found){
                break;
            }*/
        }
    }

    cv::Rect pos;
    pos.x = left;
    pos.width = right - left;
    pos.y = top;
    pos.height = bottom - top;
    //std::cout << "l: " << left << "t: " << top << "b: " << bottom << "r: " << right << std::endl;
    //pos.ok = ( l_ok && t_ok && b_ok && r_ok );
    /*if(pos.ok){
        std::cout << "Dino Bulundu: " << std::endl;
    }*/
    return pos;

}



cv::Rect DinoGame::findDinoTM(cv::Mat &I, cv::Mat templ ){
  /// Source image to display
  cv::Mat img_display, result;
  I.copyTo( img_display );

  cv::Rect ROI(0, 0, I.cols/2, I.rows);

  I = I(ROI);

  int match_method = cv::TM_CCOEFF_NORMED;

  /// Create the result matrix
  int result_cols =  I.cols - templ.cols + 1;
  int result_rows = I.rows - templ.rows + 1;

  result.create( result_rows, result_cols, CV_32FC1 );

  /// Do the Matching and Normalize
  matchTemplate( I, templ, result, match_method );
  normalize( result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

  /// Localizing the best match with minMaxLoc
  double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
  cv::Point matchLoc;

  minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

  /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
  if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
  else
    { matchLoc = maxLoc; }

  /// Show me what you got
  rectangle( img_display, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
  rectangle( result, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );

  //cv::imshow( "template match result", img_display );
  cv::Rect pos;
  pos.x = matchLoc.x;
  pos.y = matchLoc.y;
  pos.width = templ.cols;
  pos.height =  templ.rows;
  return pos;
}

