#include "bound_selector.h"
#include "ui_boundselector.h"

BoundSelector::BoundSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BoundSelector)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);


    btnSet = this->ui->btnSetBounds;
    connect(btnSet, SIGNAL(clicked(bool)), this, SLOT(btnClicked(bool)));
    setWindowOpacity(0.5);
}

void BoundSelector::mousePressEvent(QMouseEvent *event){
    mpos = event->pos();
}

void BoundSelector::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}


void BoundSelector::btnClicked(bool b){
    QRect r(
                this->pos().x(),
                this->pos().y(),
                this->width(),
                this->height()
                );
    emit BoundsSelected(r);
    this->close();
}


BoundSelector::~BoundSelector()
{
    delete btnSet;
    delete ui;
}
