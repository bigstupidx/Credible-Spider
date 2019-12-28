#include "Manager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <set>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <typeinfo>

#include "resource.h"

#include "Add13.h"
#include "Deal.h"
#include "PMove.h"
#include "ReleaseCorner.h"
#include "Restore.h"

#include "TImage.h"

#include "Firework.h"
#include "SequentialAnimation.h"
#include "ParallelAnimation.h"
#include "ValueAnimation.h"
#include "SettingAnimation.h"
#include "ValueAnimation.h"

#include <thread>
//#define _PRINT
//#define _PAUSE

using namespace std;

Manager::Manager() :poker(nullptr),
hasGUI(false), bOnThread(false), bStopThread(false), hWnd(NULL),
idCardEmpty(0), idCardBack(0), idCard1(0),
dragInfo()

{
}

Manager::Manager(int suitNum) : Manager()
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
	shared_ptr<PMove> action(new PMove(orig, dest, num));
	if (action->Do(poker))
	{
		if (hasGUI)
		{
			action->StartAnimationQuick(hWnd, bOnThread, bStopThread);
		}
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

	int suitNum, seed;
	cout << "--deal--" << endl << "input suitNum, seed: ";
	in >> suitNum >> seed;

	shared_ptr<Action> action(new Deal(suitNum, seed));
	action->Do(poker);
	if (hasGUI)
	{
		InitialImage();
		action->StartAnimation(hWnd, bOnThread, bStopThread);
	}
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
	if (hasGUI)
	{
		InitialImage();
		action->StartAnimation(hWnd, bOnThread, bStopThread);
	}
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
			int bPlayAnimation;
			in >> bPlayAnimation;
			success = AutoSolve(bPlayAnimation);
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
		if (command == "r")//release
		{
			shared_ptr<Action> action(new ReleaseCorner());
			if (success=action->Do(poker))
			{
				if (hasGUI)
				{
					action->StartAnimation(hWnd, bOnThread, bStopThread);
				}
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
				//�Զ����Ƶĳ������Ӳ��������˹���Ҫ��
				poker->score -= 2;
				poker->operation += 2;

				if (hasGUI)
				{
					//record.back()->RedoAnimation(hWnd, bOnThread, bStopThread);
					RECT rc;
					GetClientRect(hWnd, &rc);
					OnSize(rc);
					InvalidateRect(hWnd, &rc, false);
					UpdateWindow(hWnd);
				}

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

std::vector<Manager::Node> Manager::GetAllOperator(std::vector<int>& emptyIndex, std::shared_ptr<Poker> poker, const unordered_set<Poker>& states)
{
	std::vector<Manager::Node> actions;
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
			//�����ϵ���
			Card* pCardDest = &destCards.back();

			//����ƶѱ���
			for (int orig = 0; orig < poker->desk.size(); ++orig)
			{
				auto& origCards = poker->desk[orig];
				if (origCards.empty())
					continue;
				if (origCards.back().point >= pCardDest->point)
					continue;

				int num = 0;
				//������µ��Ʊ��������
				for (auto it = origCards.rbegin(); it != origCards.rend(); ++it)
				{
					num++;
					//���������ϣ������ƶ�
					if (it->point >= pCardDest->point)
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

					//it ----> pCard��Ŀ��pCard��Դit��1
					//�����ǻ�ɫ����ɫ������ֵ��������
					if (it->point + 1 == pCardDest->point)//it->suit == pCard->suit && 
					{
						shared_ptr<Poker> tempPoker(new Poker(*poker));
						shared_ptr<Action> action(new PMove(orig, dest, num));
						if (action->Do(tempPoker.get()) &&
							(states.find(*tempPoker) == states.end()))
							actions.push_back({ tempPoker->GetValue(),tempPoker,action });
						break;
					}

				}
			}
		}
	}

	//û�п�λ���Ҵ�����������
	//���뷢�Ʋ���
	if (emptyIndex.empty() && !poker->corner.empty())
	{
		shared_ptr<Poker> newPoker(new Poker(*poker));
		shared_ptr<Action> action(new ReleaseCorner());
		action->Do(newPoker.get());
		actions.push_back({ poker->GetValue() - 100,newPoker,action });
	}
	return actions;
}

bool Manager::dfs(bool& success, int& calc, vector<shared_ptr<Action>>& record, unordered_set<Poker>& states, int stackLimited, int calcLimited, bool playAnimation)
{
	if (poker->isFinished())
	{
		success = true;
		return true;
	}

	//���������������ƣ���������������
	if (poker->operation >= stackLimited || calc >= calcLimited || bStopThread)
	{
		return true;
	}

	calc++;

	auto ReleaseActions = [](vector<Node>& actions)
	{
		actions.clear();
	};



	vector<int> emptyIndex;
	shared_ptr<Poker> tempPoker(new Poker(*poker));
	vector<Node> actions = GetAllOperator(emptyIndex, tempPoker, states);

	//�Ż�����
	if (emptyIndex.empty())
		//û�п�λ
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
					if (cards.size() > 1 && cards.back().point < minPoint)
					{
						minPoint = cards.back().point;
						orig = i;
					}
				}

				//ֻ���һ�����Ʋ���λ�Ĳ���
				shared_ptr<Action> action(new PMove(orig, emptyIndex.front(), 1));
				shared_ptr<Poker> newPoker(new Poker(*poker));
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

#ifdef _PAUSE
		//�˴���ͣ
		//�����������һ��������������ֱ��stack==round��ͣ��
		if (poker->operation < round)
		{
			;
		}
		else
		{
			string s;
			getline(cin, s);
			if (s.empty())
				round = -1;
			else
				round = stoi(s);
		}
#endif

		//û���ֹ���״̬
		if (states.find(*node.poker) == states.end())
		{
			static bool bNounce = true;
			static bool bStop = false;
			node.action->Do(poker);

			if (hasGUI)
			{
				string s{ "Credible Spider (��ⲽ��=" + to_string(calc) + ")" };
				SetWindowText(hWnd, s.c_str());

				if (playAnimation)
					node.action->StartAnimation(hWnd, bNounce, bStop);
				else
				{
					RECT rc;
					GetClientRect(hWnd, &rc);
					OnSize(rc);
					InvalidateRect(hWnd, &rc, false);
					UpdateWindow(hWnd);
					Sleep(1);
				}
			}

			//����״̬
			states.insert(*node.poker);

			//push��¼
			record.push_back(node.action);

			if (dfs(success, calc, record, states, stackLimited, calcLimited, playAnimation))
			{
				//ֻ����ֹ�Ż᷵��true���������λ�÷���true���˴�������ֹ�ݹ�
				ReleaseActions(actions);
				return true;
			}

			node.action->Redo(poker);
			if (hasGUI)
			{
				if (playAnimation)
					node.action->RedoAnimation(hWnd, bNounce, bStop);
				else
				{
					RECT rc;
					GetClientRect(hWnd, &rc);
					OnSize(rc);
					InvalidateRect(hWnd, &rc, false);
					UpdateWindow(hWnd);
					Sleep(1);
				}
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

bool Manager::AutoSolve(bool playAnimation)
{
	bOnThread = true;
	bStopThread = false;

	unordered_set<Poker> states;
	bool success = false;
	int calc = 0;

	//1��ɫʱ��200���Խ��70/100����500���Խ��89/100����1000���Խ��92/100����8000���Խ��98/100��
	//2��ɫʱ��2000���Խ��23/100����8000���Խ��32/100��
	//4��ɫʱ��100000���0/6��
	int calcLimited = 2000;

	//480ʱջ��������Ա���С��480����������߱���ջ��С
	int stackLimited = 400;

	//��Ϊ�����poker������ָ�룬�ᱻ�ͷ�
	//�����½�һ��Poker�������״̬
	//δ��ɵĻ�Poker�ǿյģ���Ϊdfsֻ��true��д��result
	//Poker* result = new Poker;
	dfs(success, calc, record, states, stackLimited, calcLimited, playAnimation);

	//poker = result;

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
	bOnThread = false;
	bStopThread = false;
	return success;
}

bool Manager::CanRedo()
{
	return !record.empty();
}

void Manager::SetSoundId(int idTip, int idNoTip, int idWin)
{
	soundTip = idTip;
	soundNoTip = idNoTip;
	soundWin = idWin;
}

void Manager::SetGUIProperty(HWND hWnd, int idCardEmpty, int idCardBack, int idCard1)
{
	hasGUI = true;
	this->hWnd = hWnd;
	this->idCardEmpty = idCardEmpty;
	this->idCardBack = idCardBack;
	this->idCard1 = idCard1;

	//�ͷſ���λ
	for (auto& img : vecImgCardEmpty)
		delete img;

	//��������λ
	vecImgCardEmpty.resize(10);
	for (auto& img : vecImgCardEmpty)
	{
		delete img;
		img = new TImage(GetModuleHandle(NULL), idCardEmpty);
	}
}

void Manager::InitialImage()
{
	//ÿ���Ƽ���ͼƬ
	for (auto& vec : poker->desk)
		for (auto& card : vec)
		{
			int imageIndex = (card.suit - 1) * 13 + card.point - 1;
			shared_ptr<TImage> imgCard(new TImage(GetModuleHandle(NULL), idCard1 + imageIndex));
			shared_ptr<TImage> imgCardBack(new TImage(GetModuleHandle(NULL), idCardBack));
			card.SetImage(imgCard, imgCardBack);
		}

	//�����Ƽ���ͼƬ
	for (auto& cards : poker->corner)
		for (auto& card : cards)
		{
			int imageIndex = (card.suit - 1) * 13 + card.point - 1;
			shared_ptr<TImage> imgCard(new TImage(GetModuleHandle(NULL), idCard1 + imageIndex));
			shared_ptr<TImage> imgCardBack(new TImage(GetModuleHandle(NULL), idCardBack));
			card.SetImage(imgCard, imgCardBack);
		}

}

RECT Manager::GetCardEmptyRect(RECT rect, int index)
{
	int cardGap = (rect.right - cardWidth * 10) / 11;

	//����λλ��
	int x = cardGap + index * (cardWidth + cardGap);
	int y = border;

	return { x,y,x + cardWidth,y + cardHeight };
}

bool Manager::PtInRelease(POINT pt)
{
	bool ret = false;
	for (auto& cards : poker->corner)
	{
		auto& card = cards.front();
		int x = card.GetPos().x;
		int y = card.GetPos().y;
		RECT rect{ x,y,x + cardWidth,y + cardHeight };
		if (PtInRect(&rect, pt))
			return true;
	}
	return ret;
}

void Manager::OnSize(RECT rect)
{
	if (hasGUI == false)
		return;

	int cardGap = (rect.right - cardWidth * 10) / 11;

	if (poker == nullptr)
	{
		//ˢ�¿���λλ��
		for (int i = 0; i < vecImgCardEmpty.size(); ++i)
		{
			int x = cardGap + i * (cardWidth + cardGap);
			int y = border;

			vecImgCardEmpty[i]->pt.x = x;
			vecImgCardEmpty[i]->pt.y = y;
		}
	}
	else
	{
		//ˢ������λ��
		for (int i = 0; i < poker->desk.size(); ++i)
		{
			int x = cardGap + i * (cardWidth + cardGap);
			int y = border;

			auto& cards = poker->desk[i];
			int cardY = y;
			for (auto it = cards.begin(); it != cards.end(); ++it)
			{
				it->SetPos({ x,cardY });
				if (it->show)//�����Ƶ�y���ȱ����ƴ�
					cardY += cardGapH * 2;
				else
					cardY += cardGapH;
			}
		}

		//ˢ�¶���
		for (int i = 0; i < poker->corner.size(); ++i)
		{
			int cornerX = rect.right - cardGap - cardWidth - i * border;
			int cornerY = rect.bottom - border - cardHeight;
			for (auto& card : poker->corner[i])
			{
				card.SetPos({ cornerX,cornerY });

			}
		}

		//ˢ�������
		for (int i = 0; i < poker->finished.size(); ++i)
		{
			int x = cardGap + i * xBorder;
			int y = rect.bottom - border - cardHeight;
			for (auto& card : poker->finished[i])
				card.SetPos({ x,y });
		}
	}

}

void Manager::Draw(HDC hdc, const RECT& rect)
{

	if (poker == nullptr)
	{
		//������λ
		for (auto& imgEmpty : vecImgCardEmpty)
			imgEmpty->Draw(hdc);
	}
	else
	{
		vector<Card*> vecTopCards;

		//�������
		for (auto& cards : poker->finished)
		{
			for (auto& card : cards)
				if (card.GetZIndex() > 0)
					vecTopCards.push_back(&card);
				else
					card.Draw(hdc);
		}

		//������
		for (auto& cards : poker->corner)
			for (auto& card : cards)
			{
				if (card.GetZIndex() > 0)
					vecTopCards.push_back(&card);
				else
					card.Draw(hdc);
			}

		//������
		for (auto& vec : poker->desk)
		{
			for (auto& card : vec)
			{
				if (card.GetZIndex() > 0)
					vecTopCards.push_back(&card);
				else
					card.Draw(hdc);
			}
		}


		//�����ư�z-index��С��������
		sort(vecTopCards.begin(), vecTopCards.end(), [](const Card* c1, const Card* c2) {return c1->GetZIndex() < c2->GetZIndex(); });

		//��������
		for (auto& topCard : vecTopCards)
		{
			topCard->Draw(hdc);
		}

		//firework
		if (poker->isFinished())
		{
			int fireworkNum = 10;
			static vector<shared_ptr<Firework>> fireworks;

			//�̻������򲹳�
			while (fireworks.size() < fireworkNum)
			{
				fireworks.push_back(make_shared<Firework>(rect.right, rect.bottom));
			}

			for (auto& f : fireworks)
			{
				//�̻���������滻
				if (f->IsDead())
					f = make_shared<Firework>(rect.right, rect.bottom);
				f->Draw(hdc);
			}
		}
	}
}

bool Manager::ShowOneHint()
{
	static Poker prevPoker;
	static int i = 0;

	unordered_set<Poker> uSet;
	shared_ptr<Poker> tempPoker(make_shared<Poker>(*poker));
	vector<int> emptyIndex;
	auto actions = GetAllOperator(emptyIndex, tempPoker, uSet);

	if (actions.empty() ||
		(actions.size() == 1 && typeid(*actions.front().action) == typeid(ReleaseCorner)))
	{
		PlaySound((LPCSTR)soundNoTip, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		return false;
	}

	PlaySound((LPCSTR)soundTip, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

	//���������ִ�С����
	sort(actions.begin(), actions.end(), [](const Node& n1, const Node& n2) {return n1.value > n2.value; });

	if (prevPoker == *poker)//ͬһ����Ķ����ʾ���ƶ�����
	{
	}
	else
		i = 0;

	if (typeid(*actions[i].action) == typeid(ReleaseCorner))
	{
		i++;
		if (i == actions.size())
			i = 0;
	}

	shared_ptr<PMove> pMove = dynamic_pointer_cast<PMove>(actions[i].action);

	pMove->Do(poker);
	pMove->StartHintAnimation(hWnd, bOnThread, bStopThread);
	pMove->Redo(poker);

	RECT rect;
	GetClientRect(hWnd, &rect);
	InvalidateRect(hWnd, &rect, false);

	i++;
	if (i == actions.size())
		i = 0;

	prevPoker = *poker;

	return true;
}

int Manager::GetDestIndex(RECT clientRect, POINT pt, int origIndex)
{
	if (poker == nullptr)
		return -1;

	for (int i = 0; i < poker->desk.size(); ++i)
	{
		if (i == origIndex)
			continue;
		if (poker->desk[i].size() == 0)
		{
			RECT rect = GetCardEmptyRect(clientRect, i);
			if (PtInRect(&rect, pt))
				return i;
			else
				continue;
		}
		for (int j = 0; j < poker->desk[i].size(); ++j)
		{
			auto& card = poker->desk[i][j];
			RECT rect = { card.GetPos().x,card.GetPos().y,card.GetPos().x + cardWidth,card.GetPos().y + cardHeight };
			if (PtInRect(&rect, pt))
			{
				return i;
			}
		}
	}
	return -1;
}

void Manager::GetIndexFromPoint(int& deskIndex, int& cardIndex, POINT pt)
{
	deskIndex = -1, cardIndex = -1;
	if (poker == nullptr)
		return;

	for (int i = 0; i < poker->desk.size(); ++i)
	{
		for (int j = 0; j < poker->desk[i].size(); ++j)
		{
			auto& card = poker->desk[i][j];
			RECT rect = { card.GetPos().x,card.GetPos().y,card.GetPos().x + cardWidth,card.GetPos().y + cardHeight };
			if (PtInRect(&rect, pt))
			{
				deskIndex = i;
				cardIndex = j;
			}
		}
	}
}

bool Manager::OnLButtonDown(POINT pt)
{
	if (!hasGUI)
		return false;

	int deskIndex = -1, cardIndex = -1;
	GetIndexFromPoint(deskIndex, cardIndex, pt);
	if (deskIndex == -1)
		return false;

	int num = poker->desk[deskIndex].size() - cardIndex;
	if (!::CanPick(poker, deskIndex, num))
		return false;

	dragInfo.vecCard.clear();
	for (int i = 0; i < num; ++i)
	{
		auto& card = poker->desk[deskIndex][cardIndex + i];
		card.SetZIndex(999);
		dragInfo.vecCard.push_back({ &card,card.GetPos() - pt });
	}
	auto& topCard = poker->desk[deskIndex][cardIndex];


	dragInfo.bOnDrag = true;
	dragInfo.orig = deskIndex;
	dragInfo.num = num;
	dragInfo.cardIndex = cardIndex;

	return true;
}

void Manager::GiveUpDrag()
{
	dragInfo.bOnDrag = false;

	for (auto& pr : dragInfo.vecCard)
	{
		pr.first->SetZIndex(0);
	}
	dragInfo.vecCard.clear();

	RECT rect;
	GetClientRect(hWnd, &rect);
	OnSize(rect);
	InvalidateRect(hWnd, &rect, false);

}

bool Manager::OnMouseMove(POINT pt)
{
	if (!hasGUI)
		return false;

	if (dragInfo.bOnDrag)
	{

		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
		{
			GiveUpDrag();
		}
		else
		{
			for (auto& pr : dragInfo.vecCard)
			{
				pr.first->SetPos(pt + pr.second);

			}
			RECT rect;
			GetClientRect(hWnd, &rect);
			InvalidateRect(hWnd, &rect, false);
			return true;
		}
	}
	return false;
}

bool Manager::OnLButtonUp(POINT pt)
{
	if (!hasGUI)
		return false;

	RECT rect;
	GetClientRect(hWnd, &rect);
	if (dragInfo.bOnDrag)
	{
		int dest = GetDestIndex(rect, pt, dragInfo.orig);

		dragInfo.bOnDrag = false;

		for (auto& pr : dragInfo.vecCard)
		{
			pr.first->SetZIndex(0);
		}
		dragInfo.vecCard.clear();
		if (dest != -1 && ::CanMove(poker, dragInfo.orig, dest, dragInfo.num))
		{
			Command("m " + to_string(dragInfo.orig) + " " + to_string(dest) + " " + to_string(dragInfo.num));

			OnSize(rect);
			InvalidateRect(hWnd, &rect, false);
			return true;
		}
	}
	OnSize(rect);
	InvalidateRect(hWnd, &rect, false);
	return false;
}

bool Manager::GetIsWon()
{
	return poker->isFinished();
}

void Manager::Win()
{
	PlaySound((LPCSTR)soundWin, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

	record.clear();

	auto fun = [&]()
	{
		bOnThread = true;
		bStopThread = false;
		while (bStopThread == false)
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			InvalidateRect(hWnd, &rect, false);
			//UpdateWindow(hWnd);
			//PostMessage(hWnd, WM_PAINT, 0, 0);
			Sleep(25);//50 fps
		}
		bOnThread = false;
	};

	thread t(fun);
	t.detach();
}