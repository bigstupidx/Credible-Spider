#pragma once

#include <string>
#include <iostream>
class Card
{
public:

	int suit;//��ɫ 1 2 3 4

	int point;//���� 1-13

	bool show;//�Ƿ��ѷ���

	//���ػ�ɫ C D H S
	char getSuit() const
	{
		switch (suit)
		{
		case 1:return 'C';//÷��
		case 2:return 'D';//����
		case 3:return 'H';//����
		case 4:return 'S';//����
		default:
			throw std::string("Error:'getSuit():' Undefined suit");
		}
	}

	//��ӡ��
	//eg. [S1]
	friend std::ostream& operator<<(std::ostream& out, const Card& card);
};

bool operator==(const Card& lhs, const Card& rhs);

std::ostream& operator<<(std::ostream& out, const Card& card);