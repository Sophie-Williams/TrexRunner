#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "QObject"
#include "QTimer"
#include <Windows.h>
#include <limits.h>

enum ControllerStates{
    JUMP,
    DUCK,
    NORMAL
};

class GameController: public QObject{
    Q_OBJECT
public:
    GameController();
    ~GameController();
    void Jump();
    void Duck(int msecs);
    ControllerStates GetControllerState();
public slots:
    void KeyTimerTick();

private:
    QTimer *key_timer;
    void SendKey(WORD key, int msecs);
    void ReleaseKey();
    WORD last_key;
    static const WORD UP_KEY = 0x26;
    static const WORD DOWN_KEY = 0x28;
    ControllerStates controller_state;
    bool infinite_duck_mode = false;
};

#endif // CONTROLLER_H
