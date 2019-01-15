#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE( cv::Mat );

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    game = new DinoGame();
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
    connect(timer, SIGNAL(timeout()), this, SLOT(Tick()));
    connect(fps_timer, SIGNAL(timeout()), this, SLOT(GetFPS()));
    timer->setInterval(16);
    fps_timer->setInterval(1000);
    timer->start();
    fps_timer->start();
}

void MainWindow::GetFPS(){
    //lblFps->setText("FPS: " + QString::number(frame_count));
    current_fps = frame_count;
    std::cout << "FPS: "<< QString::number(frame_count).toStdString() << std::endl;
    frame_count = 0;
}

void MainWindow::Tick(){
    //QRect crop_rect(0, 0, 100, 100);
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

    /*pix = pix.scaled(imgView->size(),Qt::KeepAspectRatio);
    imgView->setPixmap(pix);*/
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



void MainWindow::Update(QPoint dino, QPoint blocks[], bool is_night, cv::Mat frame, bool collided){
    //imshow("deneme", frame);
    QString infoText = "";
    infoText += "FPS: " + QString::number(current_fps) + "\n";
    infoText += "Day/Night: ";
    infoText += (is_night) ? "Night" : "Day";
    infoText += "\n";
    txtInfo ->setText(infoText);
    ShowImage(mat_to_qimage_cpy(frame, QImage::Format_RGB888 /*QImage::Format_Grayscale8*/), imgInfo);

    if(collided){
        SendUp();
    }

}

void MainWindow::ShowImage(QPixmap pix, QLabel *canvas){
    pix = pix.scaled(canvas->size(),Qt::KeepAspectRatio);
    canvas->setPixmap(pix);
}


#define WINVER 0x0500
void MainWindow::SendUp(){
    INPUT ip;

    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    // Press the key
    ip.ki.wVk = 0x26; // virtual-key code for the "a" key
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));
    Sleep(5);
    // Release the key
    ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &ip, sizeof(INPUT));

}



MainWindow::~MainWindow()
{
    delete imgInfo;
    delete imgCaptured;
    delete txtInfo;
    delete timer;
    delete fps_timer;
    delete game;
    delete ui;
}
