#include "PMove.h"

#include "Poker.h"

bool PMove::canMove(const Poker *poker,int deskNum, int pos) const
{
	ISLEGAL(poker->desk,deskNum, pos);
	if (pos == poker->desk[deskNum].size() - 1) return true;
	for (int i = pos + 1; i < poker->desk[deskNum].size(); ++i)
	{
		if (poker->desk[deskNum][i - 1].show && poker->desk[deskNum][i].show &&
			poker->desk[deskNum][i - 1].point - poker->desk[deskNum][i].point == 1 &&
			poker->desk[deskNum][i - 1].suit == poker->desk[deskNum][i].suit)
			continue;
		else
			return false;
	}
	return true;
}

bool PMove::Move(Poker *poker)
{
	if (!canMove(poker,origDesk, start)) return false;
	if (poker->desk[origDesk][start].point + 1 == poker->desk[destDesk].back().point)
	{
		//������������
		for (int i = start; i < poker->desk[origDesk].size(); ++i)
			poker->desk[destDesk].push_back(poker->desk[origDesk][i]);

		//�������ߵ���
		poker->desk[origDesk].erase(poker->desk[origDesk].begin() + start, poker->desk[origDesk].end());

		//��������
		if (!poker->desk[origDesk].empty()) poker->desk[origDesk].back().show = true;

		//�鿴����
		poker->refresh(destDesk);

		poker->score--;
		poker->operation++;

		return true;
	}
	else
		return false;
}

bool PMove::Do(Poker *poker)
{
	if (bMoveMode)
		return Move(poker);
	else
		return canMove(poker, origDesk, start);
}