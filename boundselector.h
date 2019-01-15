#ifndef BOUNDSELECTOR_H
#define BOUNDSELECTOR_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QRect>

namespace Ui {
class BoundSelector;
}

class BoundSelector : public QDialog
{
    Q_OBJECT

public:
    explicit BoundSelector(QWidget *parent = nullptr);
    ~BoundSelector();

private:
    Ui::BoundSelector *ui;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QPoint mpos;
    QPushButton *btnSet;

public slots:
    void btnClicked(bool b);
signals:
    void BoundsSelected(QRect rect);
};

#endif // BOUNDSELECTOR_H
