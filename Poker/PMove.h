#pragma once

#include "Action.h"
#include <vector>
#include <memory>
#include <Windows.h>

class Restore;
class PMove:public Action
{
private:
	bool success;
	int orig;//��ʼ���
	int dest;//Ŀ����
	int num;//����
	bool shownLastCard;
	std::shared_ptr<Restore> restored;

	std::vector<POINT> vecStartPt;
	virtual void StartAnimation_inner(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation,double iDuration);
public:

	PMove(int origIndex, int destIndex,int num) :
		Action(), success(false), orig(origIndex),dest(destIndex), num(num),shownLastCard(false)
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

	virtual void StartAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
	virtual void StartAnimationQuick(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation);
	virtual void RedoAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation) override;
	void PMove::StartHintAnimation(HWND hWnd, bool& bOnAnimation, bool& bStopAnimation);
};

bool CanPick(const Poker* poker, int origIndex, int num);
bool CanMove(const Poker* poker, int origIndex, int destIndex, int num);