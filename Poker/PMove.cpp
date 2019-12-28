#include "PMove.h"

#include "Poker.h"
#include "Restore.h"
#include <algorithm>
#include <assert.h>


#include <thread>
#include "Card.h"
#include "TImage.h"
#include "ParallelAnimation.h"
#include "SequentialAnimation.h"
#include "ValueAnimation.h"
#include "SettingAnimation.h"
#include "ValueAnimation.h"
#include "CardTurnOverAnimation.h"
using namespace std;

PMove::~PMove()
{
}

//�����Ƿ�����ƶ�
//deskNum �ƶѱ��
//pos �Ʊ��
bool CanPick(const Poker* poker, int origIndex, int num)
{
	assert(origIndex >= 0 && origIndex < poker->desk.size());
	assert(num > 0 && num <= poker->desk[origIndex].size());

	//�ݴ���������
	//eg. size=10, card[9].suit
	int suit = poker->desk[origIndex].back().suit;
	int point = poker->desk[origIndex].back().point;

	//��������2���ƿ�ʼ����
	//eg. num==4, i=[0,1,2]
	for (int i = 0; i < num - 1; ++i)
	{
		//eg. size=10, up=10-[0,1,2]-2=[8,7,6]
		int index = poker->desk[origIndex].size() - i - 2;

		const Card& card = poker->desk[origIndex][index];
		if (card.suit != suit)
			return false;
		if (card.show == false)
			return false;
		if (point + 1 == card.point)
			point++;
		else
			return false;
	}
	return true;
}

bool CanMove(const Poker* poker, int origIndex, int destIndex, int num)
{
	//����ʰȡ����false
	if (!CanPick(poker, origIndex, num))
		return false;

	auto& origTopCard = (poker->desk[origIndex].end() - num);
	auto& destCards = poker->desk[destIndex];
	if (destCards.empty())
		return true;
	else
		if (origTopCard->point + 1 == destCards.back().point)//Ŀ��ѵ���������==�ƶ��ƶ���+1
			return true;
	return false;
}

bool PMove::Do(Poker* inpoker)
{
	poker = inpoker;

	//����ʰȡ����false
	if (!CanPick(poker, orig, num))
		return false;

	auto itOrigBegin = poker->desk[orig].end() - num;
	auto itOrigEnd = poker->desk[orig].end();

	auto itDest = poker->desk[dest].end();

	//Ŀ��λ��Ϊ�� ����
		//Ŀ��ѵ���������==�ƶ��ƶ���+1
	if (poker->desk[dest].empty() ||
		(itOrigBegin->point + 1 == poker->desk[dest].back().point))
	{
		//������������
		poker->desk[dest].insert(itDest, itOrigBegin, itOrigEnd);

		//����㼯
		vecStartPt.clear();
		for_each(itOrigBegin, itOrigEnd, [&](const Card& card) {vecStartPt.push_back(card.GetPos()); });

		//�������ߵ���
		poker->desk[orig].erase(itOrigBegin, itOrigEnd);

		//��������
		if (!poker->desk[orig].empty() && poker->desk[orig].back().show == false)
		{
			poker->desk[orig].back().show = true;
			shownLastCard = true;
		}
		else
			shownLastCard = false;

		poker->score--;
		poker->operation++;

		//���л���
		restored = make_shared<Restore>(dest);
		if (restored->Do(poker) == false)
			restored = nullptr;

		success = true;
		return true;
	}
	else
		return false;
}

void PMove::StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{
	StartAnimation_inner(hWnd, bOnAnimation, bStopAnimation,1.0);
}

void PMove::StartAnimationQuick(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{
	StartAnimation_inner(hWnd, bOnAnimation, bStopAnimation,0.1);
}

void PMove::StartAnimation_inner(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation,double iDuration)
{
	assert(success);

	//��������˻����¼����Ȼָ�������ǰ
	if (restored)
		restored->Redo(poker);

	SendMessage(hWnd, WM_SIZE, 0, 0);

	vector<POINT> vecEndPt;

	shared_ptr<SequentialAnimation> seq(make_shared<SequentialAnimation>());

	ParallelAnimation* para = new ParallelAnimation;

	vector<AbstractAnimation*> vecFinalAni;
	for (int i = 0; i < num; ++i)
	{
		int sz = poker->desk[dest].size();
		auto& card = poker->desk[dest][sz - num + i];

		vecEndPt.push_back(card.GetPos());

		card.SetPos(vecStartPt[i]);
		card.SetZIndex(999);

		para->Add(new ValueAnimation<Card, POINT>(&card, 500*iDuration, &Card::SetPos, vecStartPt[i], vecEndPt[i]));

		//�ָ�z-index
		vecFinalAni.push_back(new SettingAnimation<Card, int>(&card, 0, &Card::SetZIndex, 0));
	}

	//�ƶ�
	seq->Add(para);

	//��������
	if (shownLastCard)
	{
		auto& card = poker->desk[orig].back();
		seq->Add(CardTurnOverAnimation::AddBackToFrontAnimation(card));
	}

	//�ָ�z-index
	for (auto& ani : vecFinalAni)
		seq->Add(ani);

	bStopAnimation = false;
	bOnAnimation = true;
	seq->Start(hWnd, bStopAnimation);

	//�����Do�з����˻��գ���ʱ�ٽ��л���
	if (restored)
	{
		restored->Do(poker);
		restored->StartAnimation(hWnd, bOnAnimation, bStopAnimation);
	}
	bOnAnimation = false;
}

void PMove::StartHintAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{
	assert(success);

	//��������˻����¼����Ȼָ�������ǰ
	if (restored)
		restored->Redo(poker);

	SendMessage(hWnd, WM_SIZE, 0, 0);

	vector<POINT> vecEndPt;

	shared_ptr<SequentialAnimation> seq(make_shared<SequentialAnimation>());

	ParallelAnimation* para = new ParallelAnimation;
	ParallelAnimation* paraGoBack = new ParallelAnimation;

	vector<AbstractAnimation*> vecFinalAni;

	//
	if (shownLastCard)
	{
		auto& card = poker->desk[orig].back();
		card.SetShow(false);
	}
	for (int i = 0; i < num; ++i)
	{
		int sz = poker->desk[dest].size();
		auto& card = poker->desk[dest][sz - num + i];

		vecEndPt.push_back(card.GetPos());

		card.SetPos(vecStartPt[i]);
		card.SetZIndex(999);

		para->Add(new ValueAnimation<Card, POINT>(&card, 500, &Card::SetPos, vecStartPt[i], vecEndPt[i]));
		paraGoBack->Add(new ValueAnimation<Card, POINT>(&card, 500, &Card::SetPos, vecEndPt[i], vecStartPt[i]));

		//�ָ�z-index
		vecFinalAni.push_back(new SettingAnimation<Card, int>(&card, 0, &Card::SetZIndex, 0));
	}

	//�ƶ�
	seq->Add(para);
	seq->Add(paraGoBack);

	//�ָ�z-index
	for (auto& ani : vecFinalAni)
		seq->Add(ani);

	bStopAnimation = false;
	bOnAnimation = true;
	seq->Start(hWnd, bStopAnimation);

	bOnAnimation = false;
}


void PMove::RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation)
{

}

bool PMove::Redo(Poker* inpoker)
{
	assert(success);

	poker = inpoker;

	if (restored)
	{
		restored->Redo(poker);
	}

	success = false;

	poker->operation--;
	poker->score++;

	if (shownLastCard)
	{
		poker->desk[orig].back().show = false;
	}

	auto itOrigBegin = poker->desk[dest].end() - num;
	auto itOrigEnd = poker->desk[dest].end();

	auto itDest = poker->desk[orig].end();

	//�������ߵ���
	poker->desk[orig].insert(itDest, itOrigBegin, itOrigEnd);

	//������������
	poker->desk[dest].erase(itOrigBegin, itOrigEnd);

	return true;
}