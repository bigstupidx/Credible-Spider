#include "Restore.h"
#include "Poker.h"

#include <assert.h>
#include <vector>

using namespace std;

//���ض�Ӧ�ѵ��ܷ����
bool Restore::CanRestore(Poker* poker,int deskNum) const
{
	int pos = poker->desk[deskNum].size() - 1;
	int suit = poker->desk[deskNum][pos].suit;

	//i�ǵ���
	for (int i = 1; i <= 13; ++i)
	{
		//�����һ���ƿ�ʼ���������򣬻�ɫһ�� ����Ի���
		if (pos >= 0 && poker->desk[deskNum][pos].point == i && poker->desk[deskNum][pos].suit == suit)
		{
			pos--;
			continue;
		}
		else
			return false;
	}
	return true;
}

bool Restore::DoRestore(Poker* poker,int deskNum)
{
	if (CanRestore(poker,deskNum))
	{
		Oper oper;
		oper.origDeskIndex = deskNum;
		//���л���

		//�������ƣ��������һ�ŵ���13�ţ�����˳��Ϊ1-13
		vector<Card> temp(poker->desk[deskNum].rbegin(), poker->desk[deskNum].rbegin() + 13);
		poker->finished.push_back(temp);

		//ȥ���ƶѵ���13��
		poker->desk[deskNum].erase(poker->desk[deskNum].end() - 13, poker->desk[deskNum].end());

		//�����������
		if (!poker->desk[deskNum].empty() && poker->desk[deskNum].back().show == false)
		{
			poker->desk[deskNum].back().show = true;
			oper.shownLastCard = true;
		}
		else
			oper.shownLastCard = false;

		poker->score += 100;

		vecOper.push_back(oper);

		return true;
	}
	return false;
}


bool Restore::Do(Poker* inpoker)
{
	poker = inpoker;

	if (vecOper.empty())
		for (size_t i = 0; i < poker->desk.size(); ++i)
		{
			DoRestore(poker,i);
		}
	else
	{
		int deskIndex = vecOper.front().origDeskIndex;
		vecOper.clear();
		DoRestore(poker,deskIndex);
	}

	return !vecOper.empty();
}

bool Restore::Redo(Poker* inpoker)
{
	assert(vecOper.size());

	poker = inpoker;

	for (auto it = vecOper.rbegin(); it != vecOper.rend(); ++it)
	{
		poker->score -= 100;

		//����������򷭻�ȥ
		if (it->shownLastCard)
			poker->desk[it->origDeskIndex].back().show = false;

		//����ɵ��ƷŻضѵ�
		auto it1 = poker->finished.back().rbegin();
		auto it2 = poker->finished.back().rend();
		poker->desk[it->origDeskIndex].insert(poker->desk[it->origDeskIndex].end(), it1, it2);

		//��ɵ�������
		poker->finished.pop_back();
	}

	vecOper.clear();
	return true;
}