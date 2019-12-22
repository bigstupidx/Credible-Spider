#include "Manager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <memory>
#include <unordered_set>

#include "Add13.h"
#include "Deal.h"
#include "PMove.h"
#include "ReleaseCorner.h"
#include "Restore.h"

#include "TImage.h"

#include "SequentialAnimation.h"
#include "PropertyAnimation.h"
#include "SettingAnimation.h"
#include "ValueAnimation.h"

#include <thread>
//#define _PRINT

using namespace std;

Manager::Manager():poker(nullptr),imgCornerDemo(nullptr)
{
}

Manager::Manager(int suitNum):Manager()
{
	poker = new Poker;

	Deal deal(suitNum);
	deal.Do(poker);

	cout << *poker;

}

Manager::Manager(int suitNum, int seed) :Manager()
{
	poker = new Poker;

	Deal deal(suitNum, seed);
	deal.Do(poker);

	cout << *poker;

}

Manager::~Manager()
{
	delete poker;
	ReleaseRecord();

	delete imgCornerDemo;
	for (auto img : vecImgCorner)
		delete img;
	for (auto img : vecImgCardEmpty)
		delete img;
}

void Manager::ReleaseRecord()
{
	record.clear();
}

bool Manager::CanPick(Poker* poker, istream& in)
{
	int orig, num;
	cout << "--canPick--" << endl << "input orig, num: ";
	in >> orig >> num;
	cout << endl;
	cout << "canPick? ";
	if (::CanPick(poker, orig, num))
	{
		cout << "Can." << endl;
		return true;
	}
	else
	{
		cout << "Can't." << endl;
		return false;
	}
}

bool Manager::Move(Poker* poker, istream& in)
{
	int orig, dest, num;
	cout << "--move--" << endl << "input orig, dest, num: "; in >> orig >> dest >> num;
	cout << endl;
	cout << "Chose: "; poker->printCard(orig, num);
	cout << endl;
	cout << "canMove? ";
	shared_ptr<Action> action(new PMove(orig, dest, num));
	if (action->Do(poker))
	{
		record.push_back(action);
		cout << "success." << endl;
		return true;
	}
	else
	{
		cout << "failed." << endl;
		return false;
	}
}

void Manager::NewGame(istream& in)
{
	record.clear();
	delete poker;
	poker = new Poker;

	int suitNum,seed;
	cout << "--deal--" << endl << "input suitNum, seed: ";
	in >> suitNum>>seed;

	shared_ptr<Action> action(new Deal(suitNum, seed));
	action->Do(poker);
}

void Manager::NewGameRandom(istream& in)
{
	record.clear();
	delete poker;
	poker = new Poker;

	int suitNum;
	cout << "--deal--" << endl << "input suitNum: ";
	in >> suitNum;

	shared_ptr<Action> action(new Deal(suitNum));
	action->Do(poker);
}

bool Manager::Command(const string command)
{
	istringstream iss(command);
	return readIn(iss);
}

void Manager::showHelpInfo() const
{
	if (!poker)
	{
		cout << "--Command Line Help--" << endl;
		cout << "new : new game" << endl;
		cout << "newrandom : new random game" << endl;
		cout << "--Help End--" << endl << endl;
	}
	else
	{
		cout << "--Command Line Help--" << endl;
		cout << "auto" << endl;
		cout << "a add13" << endl;
		cout << "h help" << endl;
		cout << "m move" << endl;
		cout << "new : new game" << endl;
		cout << "newrandom : new random game" << endl;
		cout << "p print" << endl;
		cout << "r releaseCorner" << endl;
		cout << "redo" << endl;
		cout << "save" << endl;
		cout << "exit" << endl;
		cout << "--Help End--" << endl << endl;
	}
}

bool Manager::readIn(istream& in)
{
	showHelpInfo();
	string command;
	bool success = true;
	while (1)
	{
		cout << ">>";
		if (!(in >> command))
			break;

		if (command == "new")
		{
			NewGame(in);
			cout << *poker;
			continue;
		}
		if (command == "newrandom")
		{
			NewGameRandom(in);
			cout << *poker;
			continue;
		}

		if (poker == nullptr)
			continue;

		if (command == "auto")
		{
			AutoSolve();
			break;
			//continue;
		}
		if (command == "a")
		{
			int deskNum;
			cout << "--add13--" << endl << "input deskNum: "; in >> deskNum;
			cout << endl;
			shared_ptr<Action> action(new Add13(deskNum));
			if (action->Do(poker))
			{
				record.push_back(action);
				cout << *poker;
			}
			continue;
		}
		if (command == "h")
		{
			showHelpInfo();
			continue;
		}
		if (command == "m")
		{
			success = Move(poker, in);

			cout << *poker;
			continue;
		}
		if (command == "pick")
		{
			success = CanPick(poker, in);
			continue;
		}
		if (command == "p")
		{
			cout << *poker;
			continue;
		}
		if (command == "r")
		{
			shared_ptr<Action> action(new ReleaseCorner());
			if (action->Do(poker))
			{
				record.push_back(action);
				cout << *poker;
			}
			continue;
		}
		if (command == "redo")
		{
			if (record.size() > 0)
			{
				record.back()->Redo(poker);
				record.pop_back();
				success = true;

				cout << *poker;
			}
			else
				cout << "Can't redo." << endl;
			continue;
		}
		if (command == "save")
		{
			ofstream ofs("1.txt");
			for (auto& action : record)
			{
				ofs << *action;
			}
			ofs.close();
			continue;
		}
		if (command == "test")
		{
			vector<string> vs;
			for (auto& r : record)
				vs.push_back(r->GetCommand());

			continue;
		}
		if (command == "exit")
		{
			break;
		}

		cout << "Unknowned Command" << endl;
		throw "Error:Unknowned Command";
		//cout << ">>";
	}
	return success;
}

bool dfs(Poker& result, bool& success, int& calc, shared_ptr<Poker> poker, vector<shared_ptr<Action>>& record, unordered_set<Poker>& states, int stackLimited, int calcLimited)
{
	if (poker->isFinished())
	{
		result = *poker;
		success = true;
		return true;
	}

	if (poker->operation >= stackLimited || calc >= calcLimited)
	{
		return true;
	}

	calc++;

	struct Node
	{
		int value;
		shared_ptr<Poker> poker;
		shared_ptr<Action> action;
	};
	vector<Node> actions;

	auto ReleaseActions = [](vector<Node>& actions)
	{
		actions.clear();
	};

	vector<int> emptyIndex;
	for (int dest = 0; dest < poker->desk.size(); ++dest)
	{
		auto& destCards = poker->desk[dest];
		if (destCards.empty())
		{
			emptyIndex.push_back(dest);

			//��ǰ�ƶ�Ϊ��
			//���������ƶѣ��ѷǿյļӽ���
			for (int orig = 0; orig < poker->desk.size(); ++orig)
			{
				auto& cards = poker->desk[orig];
				if (cards.empty())
					continue;

				//�õ����ƶ�������
				//���ٻ᷵��1
				int num = 1;
				while (1)
				{
					if (num > cards.size())
					{
						num--;
						break;
					}
					if (::CanPick(poker.get(), orig, num))
					{
						num++;
					}
					else
					{
						num--;
						break;
					}
				}

				//ȫ���Ƶ���λû������
				if (num == cards.size())
					continue;

				//
				shared_ptr<Poker> newPoker(new Poker(*poker.get()));
				shared_ptr<Action> action(new PMove(orig, dest, num));
				action->Do(newPoker.get());

				if (states.find(*newPoker) == states.end())
					actions.push_back({ newPoker->GetValue(),newPoker,action });
			}
		}
		else//dest�ƶѷǿ�
		{
			//����µ���
			Card* pCard = &destCards.back();

			//����ƶѱ���
			for (int orig = 0; orig < poker->desk.size(); ++orig)
			{
				auto& origCards = poker->desk[orig];
				if (origCards.empty())
					continue;
				if (&origCards.back() == pCard)
					continue;

				int num = 0;
				//������µ��Ʊ��������
				for (auto it = origCards.rbegin(); it != origCards.rend(); ++it)
				{
					num++;
					//���������ϣ������ƶ�
					if (it->point >= pCard->point)
						break;

					//û����ʾ���ƣ������ƶ�
					if (it->show == false)
						break;

					//���ǵ�����1��
					if (it != origCards.rbegin())
					{
						auto itDown = it - 1;//��һ����
						if (itDown->point + 1 != it->point)//�������������������ƶ�
							break;
					}

					//it ----> pCard��Ŀ���Դ��1
					//�����ǻ�ɫ����ɫ������ֵ��������
					if (it->point + 1 == pCard->point)//it->suit == pCard->suit && 
					{
						shared_ptr<Poker> tempPoker(new Poker(*poker.get()));
						shared_ptr<Action> action(new PMove(orig, dest, num));
						action->Do(tempPoker.get());

						if (states.find(*tempPoker) == states.end())
							actions.push_back({ tempPoker->GetValue(),tempPoker,action });
						break;
					}

				}
			}
		}
	}

	//������������
	//���뷢�Ʋ���
	if (!poker->corner.empty())
	{
		shared_ptr<Poker> newPoker(new Poker(*poker.get()));
		shared_ptr<Action> action(new ReleaseCorner());
		action->Do(newPoker.get());
		actions.push_back({ poker->GetValue() - 100,newPoker,action });
	}

	//û�п�λ
	if (emptyIndex.empty())
		//ȥ���ȵ�ǰ���ֻ��͵�����
		for (auto it = actions.begin(); it != actions.end();)
		{
			if (typeid(*it->action) == typeid(PMove) && it->value <= poker->GetValue())
			{
				it = actions.erase(it);
			}
			else
				it++;
		}
	else
	{
		//�п�λ
		//��������������ƣ������Ƶ���λ���գ���Ϊ�п�λ���ܷ���
		if (!poker->corner.empty())
		{
			//���ȫ��˳�ƣ�����һ����С���ƹ�ȥ
			bool AllIsOrdered = true;
			for (auto& cards : poker->desk)
			{
				for (int i = 1; i < cards.size(); ++i)
				{
					if (cards[i - 1].suit != cards[i].suit || cards[i - 1].point - 1 != cards[i].point)
					{
						AllIsOrdered = false;
						break;
					}
				}
			}

			if (AllIsOrdered)
			{
				//��յ�ǰ���в���
				ReleaseActions(actions);
				int orig = 0;
				int minPoint = 14;
				//Ѱ����С����
				for (int i = 0; i < poker->desk.size(); ++i)
				{
					auto& cards = poker->desk[i];
					if (!cards.empty() && cards.back().point < minPoint)
					{
						minPoint = cards.back().point;
						orig = i;
					}
				}

				//ֻ���һ�����Ʋ���λ�Ĳ���
				shared_ptr<Action> action(new PMove(orig, emptyIndex.front(), 1));
				shared_ptr<Poker> newPoker(new Poker(*poker.get()));
				action->Do(newPoker.get());
				actions.push_back({ newPoker->GetValue(),newPoker,action });
			}
		}
	}

	//���������ִ�С����
	sort(actions.begin(), actions.end(), [](const Node& n1, const Node& n2) {return n1.value > n2.value; });

	static int round = -1;

	//��ʼ�ݹ�
	for (auto it = actions.begin(); it != actions.end();)
	{
		auto& node = *it;

#ifdef _PRINT
		cout << *poker;

		//���ԵĲ���
		cout << "Action:" << endl;
		for (auto it2 = it; it2 != actions.end(); ++it2)
			cout << *it2->action << " value:" << it2->value << endl;

		//��ʾ����
		cout << string(20, '-');
		cout << "Do:" << *node.action << endl;
#endif

		//�˴���ͣ
		//�����������һ��������������ֱ��stack==round��ͣ��
		//if (poker->operation < round)
		//{
		//	;
		//}
		//else
		//{
		//	string s;
		//	getline(cin, s);
		//	if (s.empty())
		//		round = -1;
		//	else
		//		round = stoi(s);
		//}

		//û���ֹ���״̬
		if (states.find(*node.poker) == states.end())
		{
			//����״̬
			states.insert(*node.poker);

			//push��¼
			record.push_back(node.action);

			if (dfs(result, success, calc, node.poker, record, states, stackLimited, calcLimited))
			{
				//ֻ����ֹ�Ż᷵��true���������λ�÷���true���˴�������ֹ�ݹ�
				ReleaseActions(actions);
				return true;
			}

			//pop��¼
			record.pop_back();

			it++;
		}
		else//�ѳ��ֹ���״̬
		{
#ifdef _PRINT
			cout << string(20, '-');
			cout << "No-movement:" << endl;
#endif
			//ֱ��ת����һ������
			it = actions.erase(it);
	}
}

	ReleaseActions(actions);
	return false;
}

bool Manager::AutoSolve()
{
	unordered_set<Poker> states;
	bool success = false;
	int calc = 0;

	//1��ɫʱ��200���Խ��70/100����500���Խ��89/100����1000���Խ��92/100����8000���Խ��98/100��
	//2��ɫʱ��2000���Խ��23/100����8000���Խ��32/100��
	//4��ɫʱ��100000���0/6��
	int calcLimited = 200;

	//480ʱջ��������Ա���С��480����������߱���ջ��С
	int stackLimited = 400;

	//��Ϊ�����poker������ָ�룬�ᱻ�ͷ�
	//�����½�һ��Poker�������״̬
	//δ��ɵĻ�Poker�ǿյģ���Ϊdfsֻ��true��д��result
	Poker* result = new Poker;
	dfs(*result, success, calc, shared_ptr<Poker>(poker), record, states, stackLimited, calcLimited);

	poker = result;

	//���������
	cout << "Calculation:" << calc << endl;

	if (success == true)
	{
		//�������
		cout << "Finished. Step = " << record.size() << endl;
		for (int i = 0; i < record.size(); ++i)
			cout << "[" << i << "] " << *record[i] << endl;
	}
	else
	{
		//���ʧ��ԭ��
		if (calc >= calcLimited)
			cout << "Calculation number >= " << calcLimited << endl;
		else
			cout << "Call-stack depth >= " << stackLimited << endl;
		cout << "Fail." << endl;
	}
	return success;
}

bool Manager::CanRedo()
{
	return record.size() > 1;
}

void Manager::SetImage(int idCardEmpty, int idCardBack, int idCard1)
{
	if (poker == nullptr)
	{
		vecImgCardEmpty.resize(10);
		for (auto& img : vecImgCardEmpty)
		{
			delete img;
			img = new TImage(GetModuleHandle(NULL), idCardEmpty);
		}
	}
	else
	{
		for (auto& vec : poker->desk)
			for (auto& card : vec)
			{
				int imageIndex = (card.suit - 1) * 13 + card.point - 1;
				TImage* imgCard = new TImage(GetModuleHandle(NULL), idCard1 + imageIndex);
				TImage* imgCardBack = new TImage(GetModuleHandle(NULL), idCardBack);
				card.SetImage(imgCard, imgCardBack);
			}

		imgCornerDemo = new TImage(GetModuleHandle(NULL), idCardBack);

		for (auto& img : vecImgCorner)
			delete img;
		vecImgCorner.resize(poker->corner.size());
		for (auto& img : vecImgCorner)
			img = new TImage(*imgCornerDemo);
	}
}

void Manager::OnSize(RECT rect)
{
	
	int cardGap = ((rect.right - border * 2) - cardWidth * 10) / 9;

	if (poker == nullptr)
	{
		for (int i = 0; i < vecImgCardEmpty.size(); ++i)
		{
			//
			int x = border + i * (cardWidth + cardGap);
			int y = border;

			//����λ��
			vecImgCardEmpty[i]->pt.x = x;
			vecImgCardEmpty[i]->pt.y = y;
		}
	}
	else
	{
		for (int i = 0; i < poker->desk.size(); ++i)
		{
			//
			int x = border + i * (cardWidth + cardGap);
			int y = border;

			//�����Ƶ�λ��
			auto& cards = poker->desk[i];
			int cardY = y;
			for (auto it = cards.begin(); it != cards.end(); ++it)
			{
				it->SetPos({ x,cardY });
				if (it->show)
					cardY += cardGapH * 2;
				else
					cardY += cardGapH;
			}
		}

			//ˢ�¶���
			for (int i = 0; i < vecImgCorner.size(); ++i)
			{
				int cornerX = rect.right - border - cardWidth - i * border;
				int cornerY = rect.bottom - border - cardHeight;
				vecImgCorner[i]->pt = { cornerX,cornerY };
			}
	}

}

void Manager::Draw(HDC hdc)
{

	if (poker==nullptr)
	{
		for (auto& imgEmpty : vecImgCardEmpty)
			imgEmpty->Draw(hdc);
	}
	else
	{
		//
		vector<Card*> vecTopCards;
		for (auto& vec : poker->desk)
		{
			for (auto& card : vec)
			{
				if (card.GetZIndex() == 999)
					vecTopCards.push_back(&card);
				else
					card.Draw(hdc);
			}
		}

		for (auto& topCard : vecTopCards)
		{
			topCard->Draw(hdc);
		}

		//Draw corner
		for (auto& imgCorner : vecImgCorner)
			imgCorner->Draw(hdc);
	}
}

void Manager::StartDealAnimation(HWND hWnd)
{
	SequentialAnimation *seq=new SequentialAnimation;
	POINT ptStart = vecImgCorner.back()->pt;

	vector<AbstractAnimation*> vecFinal;
	for (int i = 0; i < 54; ++i)
	{
		int deskIndex = i % 10;
		int cardIndex = i / 10;

		auto& card = poker->desk[deskIndex][cardIndex];

		//����������Ϊ���ɼ�
		card.SetVisible(false);

		//����������Ϊ�ɼ�
		SettingAnimation<Card, bool>* settingAni = new SettingAnimation<Card, bool>(&card);
		settingAni->SetDuration(0);
		settingAni->SetPFunc(&Card::SetVisible);
		settingAni->SetValue(true);
		seq->Add(settingAni);

		//�������ӽ��䵽ָ��λ��
		PropertyAnimation<Card>* ani = new PropertyAnimation<Card>(&card);
		ani->SetDuration(250);
		ani->SetPFunc(&Card::SetPos);
		ani->SetStartValue(ptStart);
		ani->SetEndValue(card.GetPos());
		seq->Add(ani);

		card.SetPos(ptStart);


		//����������Ϊ����
		card.show = false;
		
		//���10����
		if (cardIndex == poker->desk[deskIndex].size() - 1)
		{
			//���淭������ʾ
			ValueAnimation<TImage, double>* BackImageAni = new ValueAnimation<TImage, double>(&card.GetBackImage());
			BackImageAni->SetDuration(25);
			BackImageAni->SetPFunc(&TImage::SetIWidth);
			BackImageAni->SetStartValue(1.0);
			BackImageAni->SetEndValue(0.0);
			vecFinal.push_back(BackImageAni);

			//��������ʾ������
			SettingAnimation<Card, bool>* settingAni = new SettingAnimation<Card, bool>(&card);
			settingAni->SetDuration(0);
			settingAni->SetPFunc(&Card::SetShow);
			settingAni->SetValue(true);
			vecFinal.push_back(settingAni);

			//���淭����
			ValueAnimation<TImage, double>* imgAni = new ValueAnimation<TImage, double>(&card.GetImage());
			imgAni->SetDuration(25);
			imgAni->SetPFunc(&TImage::SetIWidth);
			imgAni->SetStartValue(0.0);
			imgAni->SetEndValue(1.0);
			vecFinal.push_back(imgAni);
		}
	}

	for (auto& ani : vecFinal)
		seq->Add(ani);

	//seq->Start(hWnd);
	auto fun = [](SequentialAnimation *seq,HWND hWnd)
	{
		seq->Start(hWnd);
	};

	thread t(fun,seq,hWnd);
	t.detach();
}