#include "ReleaseCorner.h"

#include "Poker.h"
#include "Restore.h"
#include <assert.h>

using namespace std;

ReleaseCorner::~ReleaseCorner()
{
	delete restored;
}

//�ͷ�һ�����½ǣ�û�м���������
bool ReleaseCorner::Do(Poker* inpoker)
{
	poker = inpoker;

	if (poker->corner.empty())
		return false;

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

	poker->operation++;

	//ˢ�¿��ܷ����
	restored = new Restore();
	if (restored->Do(poker))
		;
	else
	{
		delete restored;
		restored = nullptr;
	}

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
		delete restored;
		restored = nullptr;
	}

	vector<Card> temp;
	for (int i = 0; i < 10; ++i)
	{
		poker->desk[i].back().show = false;

		temp.push_back(poker->desk[i].back());

		poker->desk[i].pop_back();
	}

	poker->corner.push_back(temp);
	return true;
}