#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE( cv::Mat );

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    game = new DinoGame();
    connect(game, SIGNAL(GameOver()), this, SLOT(Restart()));
    qRegisterMetaType<cv::Mat>();
    connect(game, &DinoGame::ProcessFinished, this, &MainWindow::Update);
    imgCaptured = this->ui->imgCapture;
    imgInfo = this->ui->imgInfo;
    imgCaptured->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    imgInfo->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    txtInfo = this->ui->txtInfo;

    bound_selector = new BoundSelector();

    connect(bound_selector, SIGNAL(BoundsSelected(QRect)), this, SLOT(Start(QRect)));
    bound_selector->show();
}

void MainWindow::Start(QRect rect){
    bounds = rect;
    timer = new QTimer();
    fps_timer = new QTimer();
    key_release_timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(Tick()));
    connect(fps_timer, SIGNAL(timeout()), this, SLOT(GetFPS()));
    //connect(key_release_timer, SIGNAL(timeout()), this, SLOT(ReleaseKey()));
    timer->setInterval(16);
    fps_timer->setInterval(1000);
    key_release_timer->setInterval(200);
    timer->start();
    fps_timer->start();
}

void MainWindow::Restart(){
    txtInfo->setText("Game Over!");
    controller.Jump();
}

void MainWindow::GetFPS(){
    current_fps = frame_count;
    //std::cout << "FPS: "<< QString::number(frame_count).toStdString() << std::endl;
    frame_count = 0;
}

void MainWindow::Tick(){
    frame_count++;
    pix = qApp->screens().at(0)->grabWindow(
        QDesktopWidget().winId(),
        bounds.left(),
        bounds.top(),
        bounds.width(),
        bounds.height());

    cv::Mat mat = qimage_to_mat_ref(pix.toImage(), CV_8UC4);
    cv::Mat display_img;
    cvtColor(mat, display_img, CV_BGRA2RGB);
    ShowImage(mat_to_qimage_cpy(display_img, QImage::Format_RGB888), imgCaptured);
    game->LoadFrame(mat);

}

cv::Mat MainWindow::qimage_to_mat_ref(QImage const &img, int format)
{
    return cv::Mat(
        img.height(), img.width(), format,
        const_cast<uchar*>(img.bits()),
        img.bytesPerLine()
    ).clone();
}

QPixmap MainWindow::mat_to_qimage_cpy(cv::Mat const &mat, QImage::Format format)
{
    return QPixmap::fromImage(QImage(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        format
    ).copy());
}



void MainWindow::Update(QPoint dino, QPoint blocks[], bool is_night, cv::Mat frame, CollisionResult coll_data, bool on_ground, long pix_ms){
    //imshow("deneme", frame);
    QString infoText = "";
    infoText += "FPS: " + QString::number(current_fps) + "\n";
    infoText += "Day/Night: ";
    infoText += (is_night) ? "Night" : "Day";
    infoText += "\n";
    infoText += "onGround: ";
    infoText += (on_ground) ? "true" : "false";
    infoText += "\n";
    infoText += (coll_data.safe) ? "Safe: true\n" : "Safe: false\n";
    infoText += QString::number(pix_ms) + "  pixel / s\n";
    ShowImage(mat_to_qimage_cpy(frame, QImage::Format_RGB888 /*QImage::Format_Grayscale8*/), imgInfo);


    if( (coll_data.top_collision && !coll_data.bottom_collision)){
        /*on_down_press = true;
        on_up_press = false;*/
        //SendKey(DOWN_KEY);
        controller.Duck(100);
    }else if( (coll_data.top_collision || coll_data.bottom_collision)){
        /*on_up_press = true;
        on_down_press = false;*/
        //SendKey(UP_KEY);
        controller.Jump();
    }else if(coll_data.safe){
        //controller.Duck(-1);
        controller.StopKeyPress();
    }

    txtInfo->setText(infoText);


}

void MainWindow::ShowImage(QPixmap pix, QLabel *canvas){
    pix = pix.scaled(canvas->size(),Qt::KeepAspectRatio);
    canvas->setPixmap(pix);
}



MainWindow::~MainWindow()
{
    delete imgInfo;
    delete imgCaptured;
    delete txtInfo;
    delete timer;
    delete fps_timer;
    delete key_release_timer;
    delete game;
    delete ui;
}
