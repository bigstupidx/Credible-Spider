#pragma once
#include <Windows.h>

class ParallelAnimation;
class AbstractAnimation
{
protected:
	int duration;
public:
	AbstractAnimation(int duration) :duration(duration) {}
	virtual void Start(HWND hWnd,bool &StopAnimation)=0;

	//���ж����в�Ӧˢ�£�Ӧ���ɸ�����ˢ��
	virtual void StartParallel(int ms) {}

	friend class ParallelAnimation;
};

