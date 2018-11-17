#pragma once

#include <string>
#include <vector>

#include <iostream>
class Card
{
public:
	int suit;
	int point;
	bool show;
	char getSuit() const
	{
		switch (suit)
		{
		case 1:return 'H';
		case 2:return 'S';
		case 3:return 'C';
		case 4:return 'D';
		default:
			throw std::string("Error:'getSuit():' Undefined suit");
		}
	}

	void print()inline const
	{
		std::cout << (show ? "" : "[") << getSuit() << point << (show ? "" : "]");
	}
};

class Action
{

};

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
	int seed;
	int suitNum;
	std::vector<std::vector<Card>> desk;//0Ϊ������
	std::vector<std::vector<Card>> corner;//0Ϊ������
	std::vector<std::vector<Card>> finished;
public:
	Poker(int suitNum) :suitNum(suitNum){}
	std::vector<Card> genInitCard();

	void printCard(const std::vector<Card> &cards);

	void printCard(const std::vector<std::vector<Card>> &vvcards);

	void printCard();

	void deal();

	void deal(int seed);

	bool releaseCorner()
	{
		if (corner.empty()) return false;
		for (int i = 0; i < 10; ++i)
		{
			corner.back()[i].show = true;
			desk[i].push_back(corner.back()[i]);
		}
		corner.pop_back();
		return true;
	}

	bool canRestore(int deskNum)
	{

	}
};
