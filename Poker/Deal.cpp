#include "Deal.h"
#include "Poker.h"

#include <algorithm>
#include <random>
#include <Windows.h>

using namespace std;

vector<Card> Deal::genInitCard() const
{
	vector<Card> result;
	switch (suitNum)
	{
	case 1:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ 2, j });
		break;
	case 2:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ (i>3) ? 1 : 2, j });
		break;
	case 4:
		for (int i = 0; i < 8; ++i)
			for (int j = 1; j <= 13; ++j)
				result.push_back({ i % 4 + 1, j });
		break;
	default:
		throw string("Error:'genInitCard(" + to_string(suitNum) + ")");
	}
	return result;
}

void Deal::Do(Poker *poker)
{
	if (isRand)
		seed = GetTickCount();

	poker->suitNum = suitNum;
	poker->seed = seed;

	//������
	poker->desk.clear();
	poker->corner.clear();
	poker->finished.clear();

	std::default_random_engine generator;

	//����������
	auto cards = genInitCard();

	//����
	srand(seed);
	random_shuffle(cards.begin(), cards.end(), [](int i){return rand() % i; });

	//����
	int pos = 0;
	for (int i = 0; i < 4; ++i)
	{
		vector<Card> deskOne;
		for (int j = 0; j < 6; ++j)
			deskOne.push_back(cards[pos++]);
		poker->desk.push_back(deskOne);
	}

	for (int i = 0; i < 6; ++i)
	{
		vector<Card> deskOne;
		for (int j = 0; j < 5; ++j)
			deskOne.push_back(cards[pos++]);
		poker->desk.push_back(deskOne);
	}

	for (int i = 0; i < 5; ++i)
	{
		vector<Card> cornerOne;
		for (int j = 0; j < 10; ++j)
			cornerOne.push_back(cards[pos++]);
		poker->corner.push_back(cornerOne);
	}

	//
	for (auto &deskOne : poker->desk)
		deskOne.back().show = true;

	success = true;
}
