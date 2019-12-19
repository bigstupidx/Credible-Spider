#include "PMove.h"

#include "Poker.h"
#include "Restore.h"
#include <assert.h>

PMove::~PMove()
{
	delete restored;
}

//�����Ƿ�����ƶ�
//deskNum �ƶѱ��
//pos �Ʊ��
bool CanPick(const Poker *poker,int origIndex, int num)
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

	//Ŀ��ѵ���������==�ƶ��ƶ���+1
	return (poker->desk[origIndex].end() - num)->point + 1 == poker->desk[destIndex].back().point;
}

bool PMove::Do(Poker* inpoker)
{
	poker = inpoker;

	//����ʰȡ����false
	if (!CanPick(poker, orig, num)) return 
		false;

	auto itOrigBegin = poker->desk[orig].end() - num;
	auto itOrigEnd = poker->desk[orig].end();

	auto itDest = poker->desk[dest].end();

	if (poker->desk[dest].empty() ||

		//Ŀ��ѵ���������==�ƶ��ƶ���+1
		(itOrigBegin->point + 1 == poker->desk[dest].back().point))
	{
		//������������
		poker->desk[dest].insert(itDest, itOrigBegin, itOrigEnd);

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
		restored = new Restore(dest);
		if (restored->Do(poker))
			;
		else
		{
			delete restored;
			restored = nullptr;
		}

		success = true;
		return true;
	}
	else
		return false;
}

bool PMove::Redo(Poker* inpoker)
{
	assert(success);

	poker = inpoker;

	if (restored)
	{
		restored->Redo(poker);
		delete restored;
		restored = nullptr;
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