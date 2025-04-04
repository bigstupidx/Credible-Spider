#pragma once

#include "Action.h"
#include <vector>
#include <memory>
#include <Windows.h>

class Restore;
class PMove: public Action {
private:
    bool success;
    int orig;//起始编号
    int dest;//目标编号
    int num;//数量
    bool shownLastCard;
    std::shared_ptr<Restore> restored;

#ifndef _CONSOLE
    std::vector<POINT> vecStartPt;
    virtual void StartAnimation_inner(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation, double iDuration);
#endif
public:

    PMove(int origIndex, int destIndex, int num) :
        Action(), success(false), orig(origIndex), dest(destIndex), num(num), shownLastCard(false) {
    }

    virtual ~PMove() override;

    int GetNum() const {
        return num;
    }

    //如果mode==true则真移动，否则不移动，只返回是否可移动
    virtual bool Do(Poker* inpoker) override;

    virtual bool Redo(Poker* inpoker) override;

    std::string GetCommand()const override {
        using namespace std;
        return string("m ") + to_string(orig) + " " + to_string(dest) + " " + to_string(num);
    }

#ifndef _CONSOLE
    virtual void StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
    virtual void StartAnimationQuick(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation);
    virtual void RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
    void StartHintAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation);
#endif
};

int GetPickCount(const Poker* poker, int origIndex);
bool CanPick(const Poker* poker, int origIndex, int num);
bool CanMove(const Poker* poker, int origIndex, int destIndex, int num);