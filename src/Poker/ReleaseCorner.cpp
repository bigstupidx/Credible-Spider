#include "ReleaseCorner.h"

#include "Poker.h"
#include "Restore.h"
#include <assert.h>

#include <algorithm>
#include <numeric>
#include <thread>
#include "resource.h"
#include "Card.h"

#ifndef _CONSOLE
#include "TImage.h"
#include "SequentialAnimation.h"
#include "ValueAnimation.h"
#include "SettingAnimation.h"
#include "ValueAnimation.h"
#include "CardTurnOverAnimation.h"
#endif

using namespace std;

ReleaseCorner::~ReleaseCorner() {
}

//�ͷ�һ�����½ǣ�����������
bool ReleaseCorner::Do(Poker* inpoker) {
    poker = inpoker;

    //������û��
    if (poker->corner.empty())
        return false;

    ////�п�λ���ܷ��ƣ���������С��10�Ų�������
    //int sum = 0;
    //bool hasEmpty = false;
    //for (auto& cards : poker->desk) {
    //    sum += cards.size();
    //    if (cards.empty())
    //        hasEmpty = true;
    //}
    //if (hasEmpty && sum >= 10)
    //    return false;

#ifndef _CONSOLE
    //ȡ�ý���������
    if (poker->hasGUI)
        ptStart = poker->corner.back().back().GetPos();
#endif

    //����һ����������
    for (int i = 0; i < 10; ++i) {
        //����������
        poker->corner.back()[i].show = true;

        //����ѵ�����
        poker->desk[i].push_back(poker->corner.back()[i]);

    }

    //ȥ��һ��������
    poker->corner.pop_back();
    success = true;

    poker->score--;
    poker->operation++;

    //���л���
    restored = make_shared<Restore>();
    if (restored->Do(poker) == false)
        restored = nullptr;

    return true;
}

bool ReleaseCorner::Redo(Poker* inpoker) {
    assert(success);

    poker = inpoker;

    //��������
    if (restored) {
        restored->Redo(poker);
    }

    poker->score++;
    poker->operation--;

    //����10����
    vector<Card> temp;
    for (int i = 0; i < 10; ++i) {
        //��Ϊ����
        poker->desk[i].back().show = false;

        //����
        temp.push_back(poker->desk[i].back());

        //������ȡ��
        poker->desk[i].pop_back();
    }

    poker->corner.push_back(temp);
    return true;
}


#ifndef _CONSOLE
void ReleaseCorner::StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) {
    assert(poker->hasGUI);
    assert(success);
    //��������˻����¼����Ȼָ�������ǰ
    if (restored)
        restored->Redo(poker);

    //ˢ���Ƶ����λ��
    SendMessage(hWnd, WM_SIZE, 0, 0);

    shared_ptr<SequentialAnimation> seq(make_shared<SequentialAnimation>());

    vector<AbstractAnimation*> vecFinalAni;
    for (int i = 0; i < 10; ++i) {
        auto& card = poker->desk[i].back();

        //����������Ϊ�ɼ�
        card.SetVisible(true);

        //��һ����������
        card.SetZIndex(999 - i);

        //����������Ϊ�ɼ�
        seq->Add(new SettingAnimation<Card, bool>(&card, 0, &Card::SetVisible, true));

        //�������ӽ��䵽ָ��λ��
        seq->Add(new ValueAnimation<Card, POINT>(&card, 25, &Card::SetPos, ptStart, card.GetPos()));

        vecStartPos.push_back(ptStart);
        vecEndPos.push_back(card.GetPos());

        card.SetPos(ptStart);

        //�ӱ��淭����
        auto temp = CardTurnOverAnimation::AddBackToFrontAnimation(card);
        vecFinalAni.insert(vecFinalAni.end(), temp.begin(), temp.end());

        //�������ָ�z-index
        seq->Add(new SettingAnimation<Card, int>(&card, 0, &Card::SetZIndex, 0));
    }

    seq->Add(vecFinalAni);

    if (enableSound) {
        //
        int msAll = 75 * 10;
        int times = msAll / 125 + 1;
        auto play = [&]() {
            PlaySound((LPCSTR)soundDeal, GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
        };
        while (times--) {
            thread t(play);
            t.detach();
        }
    }

    bStopAnimation = false;
    bOnAnimation = true;
    seq->Start(hWnd, bStopAnimation);

    //�����Do�з����˻��գ���ʱ�ٽ��л���
    if (restored) {
        restored->Do(poker);
        restored->StartAnimation(hWnd, bOnAnimation, bStopAnimation);
    }
    bOnAnimation = false;
}

void ReleaseCorner::RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) {
    assert(poker->hasGUI);
    SequentialAnimation* seq = new SequentialAnimation;

    for (int i = 0; i < 10; ++i) {
        //���һ��ÿ����
        auto& card = poker->corner.back()[i];

        //����������Ϊ�ɼ�
        card.SetVisible(true);

        //����������Ϊ�����Ƶ�λ��
        card.SetPos(vecEndPos[i]);

        //����ʱ����Ϊ����
        card.SetZIndex(999);

        //�����淭�ر���
        seq->Add(CardTurnOverAnimation::AddFrontToBackAnimation(card));

        //�������ӽ��䵽ָ��λ��
        seq->Add(new ValueAnimation<Card, POINT>(&card, 25, &Card::SetPos, vecEndPos[i], vecStartPos[i]));

        //�ָ�z-index
        seq->Add(new SettingAnimation<Card, int>(&card, 0, &Card::SetZIndex, 0));
    }

    vecStartPos.clear();
    vecEndPos.clear();

    //
    int msAll = 75 * 10;
    int times = msAll / 125 + 1;
    auto play = []() {
        PlaySound((LPCSTR)IDR_WAVE_DEAL, GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
    };
    while (times--) {
        thread t(play);
        t.detach();
    }

    bStopAnimation = false;
    bOnAnimation = true;
    seq->Start(hWnd, bStopAnimation);
    delete seq;
    bOnAnimation = false;
    SendMessage(hWnd, WM_SIZE, 0, 0);
    RECT rc;
    GetClientRect(hWnd, &rc);
    InvalidateRect(hWnd, &rc, false);
    UpdateWindow(hWnd);
}
#endif