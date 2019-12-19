#include "Deal.h"
#include "Poker.h"

#include <algorithm>
#include <random>
#include <Windows.h>

using namespace std;

//����1ά���飬����ɫ����1-13�㣬��8*13=104��
vector<Card> Deal::genInitCard() const
{
	vector<Card> result;
	switch (suitNum)
	{
	case 1:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ 4, j });//1����ɫ������
		break;
	case 2:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ (i>3) ? 3 : 4, j });//2����ɫ�����ң�����
		break;
	case 4:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ i % 4 + 1, j });//4����ɫ
		break;
	default:
		throw string("Error:'genInitCard(" + to_string(suitNum) + ")");
	}
	return result;
}

bool Deal::Do(Poker* inpoker)
{
	poker = inpoker;

	//������
	poker->desk.clear();
	poker->corner.clear();
	poker->finished.clear();

	std::default_random_engine e;

	//����������
	auto cards = genInitCard();

	//����
	e.seed(seed);
	random_shuffle(cards.begin(), cards.end(), [&](int i){return e() % i; });

	//����
	int pos = 0;

	//4��6�ŵ�=24
	for (int i = 0; i < 4; ++i)
	{
		vector<Card> deskOne;
		for (int j = 0; j < 6; ++j)
			deskOne.push_back(cards[pos++]);
		poker->desk.push_back(deskOne);
	}

	//6��5�ŵ�=30
	for (int i = 0; i < 6; ++i)
	{
		vector<Card> deskOne;
		for (int j = 0; j < 5; ++j)
			deskOne.push_back(cards[pos++]);
		poker->desk.push_back(deskOne);
	}

	//5�� ������=50
	for (int i = 0; i < 5; ++i)
	{
		vector<Card> cornerOne;
		for (int j = 0; j < 10; ++j)
			cornerOne.push_back(cards[pos++]);
		poker->corner.push_back(cornerOne);
	}

	//ÿ�������������
	for (auto &deskOne : poker->desk)
		deskOne.back().show = true;

	poker->score = 500;
	poker->operation = 0;

	return true;
}

bool Deal::Redo(Poker* inpoker)
{
	poker = inpoker;

	poker->desk.clear();
	poker->corner.clear();
	poker->finished.clear();

	return true;
}