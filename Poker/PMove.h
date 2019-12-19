#pragma once

#include "Action.h"

class Restore;
class PMove:public Action
{
private:
	bool success;
	int orig;//��ʼ���
	int dest;//Ŀ����
	int num;//����
	bool shownLastCard;
	Restore* restored;
public:

	PMove(int origIndex, int destIndex,int num) :
		Action(), success(false), orig(origIndex),dest(destIndex), num(num),shownLastCard(false),restored(nullptr)
	{

	}

	virtual ~PMove() override;

	int GetNum() const
	{
		return num;
	}

	//���mode==true�����ƶ��������ƶ���ֻ�����Ƿ���ƶ�
	virtual bool Do(Poker* inpoker) override;

	virtual bool Redo(Poker* inpoker) override;

	std::string GetCommand()const override
	{
		using namespace std;
		return string("m ") + to_string(orig) + " " + to_string(dest) + " " + to_string(num);
	}
};

bool CanPick(const Poker* poker, int origIndex, int num);
bool CanMove(const Poker* poker, int origIndex, int destIndex, int num);