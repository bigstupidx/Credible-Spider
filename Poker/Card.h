#pragma once

#include <string>
#include <iostream>
class Card
{
public:
	int suit;//��ɫ
	int point;
	bool show;
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

	void print()inline const
	{
		std::cout << (show ? "" : "[") << getSuit() << point << (show ? "" : "]");
	}
};
