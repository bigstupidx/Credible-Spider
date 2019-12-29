#pragma once
#include "Action.h"
#include <vector>
#include <Windows.h>

class Poker;
class Restore :
	public Action
{
private:

	struct Oper
	{
		int origDeskIndex;//����Դ�ѵ����
		bool shownLastCard;//���պ�����Ƿ񷭳�
		std::vector<POINT> vecStartPt;
		POINT ptEnd;
	};

	//�ѻ��ճɹ��Ĳ���
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

#ifndef _CONSOLE
	virtual void StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
	virtual void RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
#endif
};

