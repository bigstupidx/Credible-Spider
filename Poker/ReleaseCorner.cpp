#include "ReleaseCorner.h"

#include "Poker.h"

ReleaseCorner::ReleaseCorner() 
{
	success = false;
}


ReleaseCorner::~ReleaseCorner()
{
}

bool ReleaseCorner::releaseCorner(Poker *poker)
{
	if (poker->corner.empty()) return false;

	//����10����
	for (int i = 0; i < 10; ++i)
	{
		//����
		poker->corner.back()[i].show = true;

		//����
		poker->desk[i].push_back(poker->corner.back()[i]);

		//ˢ�¿��ܷ����
		poker->refresh(i);
	}
	poker->corner.pop_back();
	return true;
}

//�ͷ�һ�����½ǣ�û�м���������
void ReleaseCorner::Do(Poker *poker)
{
	success = releaseCorner(poker);
}