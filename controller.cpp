#include "controller.h"

#define WINVER 0x0500

GameController::GameController(){
    key_timer = new QTimer();
    connect(key_timer, SIGNAL(timeout()), this, SLOT(KeyTimerTick()));
    //controller_state = ControllerStates::NORMAL;
    infinite_duck_mode = false;
}

void GameController::Jump(){
    SendKey(UP_KEY, 10);
    //controller_state = ControllerStates::JUMP;
    infinite_duck_mode = false;
}

void GameController::Duck(int msecs){
    if(infinite_duck_mode){
        return;
    }

    if(msecs == -1){
        infinite_duck_mode = true;
        msecs = std::numeric_limits<int>::max();
    }else{
        infinite_duck_mode = false;
    }

    SendKey(DOWN_KEY, msecs);
    //controller_state = ControllerStates::DUCK;
}

void GameController::StopKeyPress(){
    ReleaseKey();
    key_timer->stop();
}

void GameController::SendKey(WORD key, int msecs){
    //key_release_timer->stop();
    //ReleaseKey();

    INPUT ip;

    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = key;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));
    last_key = key;

    key_timer->stop();
    key_timer->setInterval(msecs);
    key_timer->start();

}

void GameController::ReleaseKey(){
    INPUT ip;

    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = last_key;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void GameController::KeyTimerTick(){
    ReleaseKey();
    key_timer->stop();
}

GameController::~GameController(){
    delete key_timer;
}
