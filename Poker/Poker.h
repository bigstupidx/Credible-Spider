#pragma once

#include <string>
#include <vector>

#include "Card.h"

#ifdef _DEBUG
#include <assert.h>
#define ISLEGAL(desk,deskNum,pos) assert((pos) >= 0);assert((deskNum)<(desk).size());assert((pos) < (desk)[(deskNum)].size());
#else
#define ISLEGAL(desk,deskNum,pos) ;
#endif

class Poker
{
	/*
	�����Ϲ�10���ƣ�ǰ4��6�ţ���6��5�ţ���6*4+5*6=54��

	���乲5����ÿ��10�ţ���10*5=50��

	�ܼ�104�ţ�104=13*8��Ϊ������ȥ����С���õ�

	heart ����
	spade ����
	club ÷��
	diamond ����
	*/
private:

	//���ض�Ӧ�ѵ��ܷ����
	bool canRestore(int deskNum) const;


public:
	int seed;//����
	int suitNum;//��ɫ
	int score;//����
	int operation;//��������
	bool isFinished();
	std::vector<std::vector<Card>> desk;//0Ϊ������
	std::vector<std::vector<Card>> corner;//0Ϊ������
	std::vector<std::vector<Card>> finished;

	void printCard(const std::vector<Card> &cards) const;
	void printCard(const std::vector<std::vector<Card>> &vvcards) const;
	void printCard(int deskNum, int pos) const;
	void printCard() const;

	void refresh();
	void Poker::refresh(int deskNum);


	//void testCanMove() const;

};
