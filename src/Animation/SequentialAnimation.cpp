#include "SequentialAnimation.h"

void SequentialAnimation::Start(HWND hWnd, bool& StopAnimation) {
    while (!animations.empty()) {
        //��⵽�������
        if (StopAnimation == true)
            break;

        animations.front()->Start(hWnd, StopAnimation);

        delete animations.front();
        animations.pop();
    }

    //�����ƺ�����
    while (!animations.empty()) {
        delete animations.front();
        animations.pop();
    }
}