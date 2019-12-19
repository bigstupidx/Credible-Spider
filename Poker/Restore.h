#pragma once
#include "Action.h"
#include <vector>

class Poker;
class Restore :
	public Action
{
private:
	struct Oper
	{
		int origDeskIndex;
		bool shownLastCard;
	};

	std::vector<Oper> vecOper;

	//���ض�Ӧ�ѵ��ܷ����
	bool CanRestore(Poker* poker,int deskNum) const;
	bool DoRestore(Poker* poker,int deskNum);
public:

	//�����жѵ��пɻ��յ���������
	Restore() : Action(), vecOper() { }

	//����Ӧ�ѵ��ܻ��������
	Restore(int deskNum) : Action(), vecOper({ { deskNum,false} }) {}

	virtual bool Do(Poker* inpoker) override;
	virtual bool Redo(Poker* inpoker) override;

	std::string GetCommand()const override
	{
		return std::string("restore");
	}
};

