#pragma once

#include "Action.h"

class Restore;
class Add13 : public Action {
private:
    int deskIndex;
    Restore *restored;
public:
    Add13(int deskIndex): Action(), deskIndex(deskIndex), restored(nullptr) { }
    virtual ~Add13()override;

    virtual bool Do(Poker *inpoker) override;
    virtual bool Redo(Poker* inpoker) override;
    std::string GetCommand() const override {
        return std::string("a ") + std::to_string(deskIndex);
    }

#ifndef _CONSOLE
    virtual void StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override {}
    virtual void RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override {}
#endif
};