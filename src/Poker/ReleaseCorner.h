#pragma once

#include "Action.h"

#include <memory>
#include <windows.h>

class Restore;
class ReleaseCorner: public Action {
private:
    bool success;
    std::shared_ptr<Restore> restored;
    bool enableSound;
    int soundDeal;

#ifndef _CONSOLE
    POINT ptStart;
    std::vector<POINT> vecStartPos, vecEndPos;
#endif

public:
    ReleaseCorner(bool enableSound, int soundDeal) : Action(),
        success(false), restored(nullptr)
#ifndef _CONSOLE
        , ptStart({ 0, 0 })
#endif
    , enableSound(enableSound), soundDeal(soundDeal)
    {}

    virtual ~ReleaseCorner() override;

    virtual bool Do(Poker* inpoker) override;
    virtual bool Redo(Poker* inpoker) override;

    std::string GetCommand()const {
        return std::string("r");
    }

#ifndef _CONSOLE
    virtual void StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
    virtual void RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
#endif
};

