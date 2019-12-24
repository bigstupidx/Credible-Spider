#include "ReleaseCorner.h"

#include "Poker.h"
#include "Restore.h"
#include <assert.h>

#include <thread>
#include "Card.h"
#include "TImage.h"
#include "SequentialAnimation.h"
#include "ValueAnimation.h"
#include "SettingAnimation.h"
#include "ValueAnimation.h"
#include "CardTurnOverAnimation.h"
using namespace std;

ReleaseCorner::~ReleaseCorner()
{
}

//�ͷ�һ�����½ǣ�����������
bool ReleaseCorner::Do(Poker* inpoker)
{
	poker = inpoker;

	if (poker->corner.empty())
		return false;

	if (poker->corner.back().back().HasImage())
		ptStart = poker->corner.back().back().GetPos();

	//����һ����������
	for (int i = 0; i < 10; ++i)
	{
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

bool ReleaseCorner::Redo(Poker* inpoker)
{
	assert(success);

	poker = inpoker;

	//
	if (restored)
	{
		restored->Redo(poker);
	}

	//����10����
	vector<Card> temp;
	for (int i = 0; i < 10; ++i)
	{
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



void ReleaseCorner::StartAnimation(bool blocking,HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{
	//��������˻����¼����Ȼָ�������ǰ
	if (restored)
		restored->Redo(poker);

	//ˢ���Ƶ����λ��
	SendMessage(hWnd, WM_SIZE, 0, 0);

	SequentialAnimation* seq = new SequentialAnimation;

	vector<AbstractAnimation*> vecFinalAni;
	for (int i = 0; i < 10; ++i)
	{
		auto& card = poker->desk[i].back();

		//����������Ϊ�ɼ�
		card.SetVisible(true);

		//��һ����������
		card.SetZIndex(999-i);

		//����������Ϊ�ɼ�
		seq->Add(new SettingAnimation<Card, bool>(&card,0,&Card::SetVisible,true));

		//�������ӽ��䵽ָ��λ��
		seq->Add(new ValueAnimation<Card,POINT>(&card,25,&Card::SetPos,ptStart,card.GetPos()));

		vecStartPos.push_back(ptStart);
		vecEndPos.push_back(card.GetPos());

		card.SetPos(ptStart);

		//�ӱ��淭����
		auto temp=CardTurnOverAnimation::AddBackToFrontAnimation(card);
		vecFinalAni.insert(vecFinalAni.end(), temp.begin(), temp.end());

		//�������ָ�z-index
		seq->Add(new SettingAnimation<Card, int>(&card,0,&Card::SetZIndex,0));
	}

	seq->Add(vecFinalAni);

	//
	int msAll = 75*10;
	int times = msAll / 125 + 1;
	auto play = []()
	{
		PlaySound((LPCSTR)115, GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
	};
	while (times--)
	{
		thread t(play);
		t.detach();
	}

	//seq->Start(hWnd);
	auto fun = [&](SequentialAnimation* seq, HWND hWnd)
	{
		bStopAnimation = false;
		bOnAnimation = true;
		seq->Start(hWnd, bStopAnimation);
		delete seq;
		bOnAnimation = false;

		//�����Do�з����˻��գ���ʱ�ٽ��л���
		if (restored)
		{
			restored->Do(poker);
			restored->StartAnimation(true, hWnd, bOnAnimation, bStopAnimation);
		}
	};

	if (blocking)
	{
		//thread t(fun, seq, hWnd);
		//t.join();
		fun(seq, hWnd);
	}
	else
	{
	thread t(fun, seq, hWnd);
		t.detach();
	}
}

void ReleaseCorner::RedoAnimation(bool blocking,HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{
	SequentialAnimation* seq = new SequentialAnimation;

	for (int i = 0; i < 10; ++i)
	{
		//���һ��ÿ����
		auto& card = poker->corner.back()[i];

		//����������Ϊ�ɼ�
		card.SetVisible(true);

		//����������Ϊ�����Ƶ�λ��
		card.SetPos(vecEndPos[i]);

		//�����淭�ر���
		seq->Add(CardTurnOverAnimation::AddFrontToBackAnimation(card));

		//�������ӽ��䵽ָ��λ��
		seq->Add(new ValueAnimation<Card, POINT>(&card,25,&Card::SetPos,vecEndPos[i],vecStartPos[i]));

	}

	vecStartPos.clear();
	vecEndPos.clear();

	//
	int msAll = 75 * 10;
	int times = msAll / 125 + 1;
	auto play = []()
	{
		PlaySound((LPCSTR)115, GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
	};
	while (times--)
	{
		thread t(play);
		t.detach();
	}

	//seq->Start(hWnd);
	auto fun = [&](SequentialAnimation* seq, HWND hWnd)
	{
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
	};

	if (blocking)
	{
		fun(seq,hWnd);
	}
	else
	{
	thread t(fun, seq, hWnd);
		t.detach();
	}
}