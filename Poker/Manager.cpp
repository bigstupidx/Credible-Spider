#include "Manager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <unordered_set>

#include "Add13.h"
#include "Deal.h"
#include "PMove.h"
#include "ReleaseCorner.h"
#include "Restore.h"

using namespace std;

Manager::Manager(int suitNum)
{
	poker = new Poker;

	Deal deal(suitNum);
	deal.Do(poker);

	cout << *poker;
}

Manager::Manager(int suitNum, int seed)
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
}

void Manager::ReleaseRecord()
{
	for (auto& r : record)
		delete r;
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
	Action* action = new PMove(orig, dest, num);
	if (action->Do(poker))
	{
		record.push_back(action);
		cout << "success." << endl;
		return true;
	}
	else
	{
		delete action;
		cout << "failed." << endl;
		return false;
	}
}

bool Manager::Command(const string command)
{
	istringstream iss(command);
	return readIn(iss);
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
		if (command == "auto")
		{
			autoSolve();
			continue;
		}
		if (command == "a")
		{
			int deskNum;
			cout << "--add13--" << endl << "input deskNum: "; in >> deskNum;
			cout << endl;
			Action* action = new Add13(deskNum);
			if (action->Do(poker))
			{
				record.push_back(action);
				cout << *poker;
			}
			else
				delete action;
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
			Action* action = new ReleaseCorner();
			if (action->Do(poker))
			{
				record.push_back(action);
				cout << *poker;
			}
			else
				delete action;
			continue;
		}
		if (command == "redo")
		{
			if (record.size() > 0)
			{
				record.back()->Redo(poker);
				delete record.back();
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

void Manager::showHelpInfo() const
{
	cout << "--Command Line Help--" << endl;
	cout << "auto" << endl;
	cout << "a add13" << endl;
	cout << "h help" << endl;
	cout << "m move" << endl;
	cout << "p print" << endl;
	cout << "r releaseCorner" << endl;
	cout << "redo" << endl;
	cout << "save" << endl;
	cout << "exit" << endl;
	cout << "--Help End--" << endl << endl;
}

bool dfs(Poker* poker,vector<Action*> record, unordered_set<Poker>& states)
{
	if (poker->isFinished())
	{
		return true;
	}

	struct Node
	{
		int value;
		Poker* poker;
		Action* action;
	};
	vector<Node> actions;

	auto ReleaseActions = [](vector<Node> &actions)
	{
		for (auto& node : actions)
		{
			delete node.poker;
			delete node.action;
		}
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

				int num = 1;
				while (1)
				{
					if (num > cards.size())
					{
						num--;
						break;
					}
					if (::CanPick(poker, orig, num))
					{
						num++;
					}
					else
					{
						num--;
						break;
					}
				}

				//
				Poker* newPoker = new Poker(*poker);
				Action* action = new PMove(orig, dest, num);
				action->Do(newPoker);

				if (states.find(*newPoker) == states.end())
					actions.push_back({ newPoker->GetValue(),newPoker,action });
				else
				{
					delete newPoker;
					delete action;
				}
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
						Poker* tempPoker = new Poker(*poker);
						Action* action = new PMove(orig, dest, num);
						action->Do(tempPoker);

						if (states.find(*tempPoker) == states.end())
							actions.push_back({ tempPoker->GetValue(),tempPoker,action });
						else
						{
							delete tempPoker;
							delete action;
						}
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
		Poker* newPoker = new Poker(*poker);
		Action* action = new ReleaseCorner();
		action->Do(newPoker);
		actions.push_back({ newPoker->GetValue(),newPoker,action });
	}

	//û�п�λ
	if (emptyIndex.empty())
		//ȥ������ֵ�����
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
		//���������������
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
				ReleaseActions(actions);
				int orig = 0;
				int minPoint = 14;
				for (int i = 0; i < poker->desk.size(); ++i)
				{
					auto& cards = poker->desk[i];
					if (!cards.empty() && cards.back().point < minPoint)
					{
						minPoint = cards.back().point;
						orig = i;
					}
				}

				Action* action = new PMove(orig, emptyIndex.front(), 1);
				Poker* newPoker = new Poker(*poker);
				action->Do(newPoker);
				actions.push_back({ newPoker->GetValue(),newPoker,action });
			}
		}
	}

	//sort
	sort(actions.begin(), actions.end(), [](const Node& n1, const Node& n2) {return n1.value > n2.value; });

	static int round = 0;

	//��ʼ�ݹ�
	for (auto it=actions.begin();it!=actions.end();)
	{
		auto& node = *it;

		cout << "Action:" << endl;
		for (auto it2=it;it2!=actions.end();++it2)
			cout << *it2->action << " value:" << it2->value << endl;

		//��ʾ����
		cout << string(20, '-');
		cout << "Do:" << *node.action << endl;

		if (poker->operation < round)
		{
			;
		}
		else
		{
			string s;
			getline(cin, s);
			if (s.empty())
				round = 0;
			else
				round = stoi(s);
			//system("pause");
		}

		//û���ֹ���״̬
		if (states.find(*node.poker) == states.end())
		{
			//����״̬
			states.insert(*node.poker);

			cout << *node.poker;

			record.push_back(node.action);

			if (dfs(node.poker,record, states))
			{
				ReleaseActions(actions);
				return true;
			}

			record.pop_back();

			delete node.poker;
			delete node.action;

			it++;
		}
		else//�ѳ��ֹ���״̬
		{
			delete node.poker;
			delete node.action;

			cout << string(20, '-');
			cout << "No-movement:" << endl;
			//cout << *poker;

			it = actions.erase(it);
		}
	}

	ReleaseActions(actions);
	return false;
}

void Manager::autoSolve()
{
	unordered_set<Poker> states;

	dfs(poker,record, states);

	cout << "Finished. Step = " << record.size() << endl;
	for (int i = 0; i < record.size(); ++i)
		cout << "[" << i << "] " << *record[i] << endl;
}

bool Manager::canRedo()
{
	return record.size() > 1;
}



Action* Manager::GetLastAct()
{
	if (record.empty())
		return NULL;
	else
		return record.back();
}