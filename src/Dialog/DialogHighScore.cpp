#include "DialogHighScore.h"

#include <string>
#include "Configuration.h"

#include "DialogEvaluate.h"

using namespace std;

extern Configuration config;

void DialogHighScore::FillListView(TListView& listView,TStatic &staticHighScore,TStatic &staticWinNum, const std::vector<std::shared_ptr<Configuration::Record>>& record)
{

	int highScore = 0;
	unsigned int highScoreSeed = 0;
	int winNum = 0;
	int loseNum = 0;
	time_t t;
	for (auto& rec : record)
	{
		//�б�������Ŀ
		listView.AddItem(rec->ToVecString());

		//������߷�
		if (rec->highScore > highScore)
		{
			highScore = rec->highScore;
			highScoreSeed = rec->seed;
			t = rec->time;
		}

		//����ʤ��
		if (rec->solved)
			winNum++;
		else
			loseNum++;
	}

	string sTime;
	if (!record.empty())
		sTime = "ʱ��="+time_tToString(t);

	staticHighScore.SetText("��߼�¼��  " + to_string(highScore) + "  ( "+sTime+" ����=" + to_string(highScoreSeed) + " )");

	string sRate;
	if (winNum + loseNum == 0)
		sRate = "0";
	else
		sRate = to_string(double(winNum) / (winNum + loseNum) * 100.0);
	staticWinNum.SetText("ʤ��  " + to_string(winNum) + "    ����" + to_string(loseNum) + "    ʤ�ʣ�" + sRate + "%");
}

void DialogHighScore::UpdateListViewAndStatic()
{
	for (int i = 0; i < 3; ++i)
	{
		vecListView[i].DeleteAllItems();
		FillListView(vecListView[i], vecStaticHighScore[i], vecStaticWinNum[i], config.record[i]);
	}
}

LRESULT DialogHighScore::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//������߼�¼
	vecStaticHighScore.resize(3);
	vecStaticHighScore[0].LinkControl(m_hWnd, IDC_STATIC_HIGHSCORE);
	vecStaticHighScore[2] = vecStaticHighScore[1] = vecStaticHighScore[0];

	//���û�ʤ
	vecStaticWinNum.resize(3);
	vecStaticWinNum[0].LinkControl(m_hWnd, IDC_STATIC_WINNUM);
	vecStaticWinNum[2] = vecStaticWinNum[1] = vecStaticWinNum[0];

	//�б��
	vecListView.resize(3);
	vecListView[0].LinkControl(m_hWnd, IDC_LIST_RECORD);
	vecListView[0].SetFullRowSelect(true);

	vecListView[1] = vecListView[0];
	vecListView[2] = vecListView[0];

	for (auto& listView : vecListView)
	{
		//����б��ǩ
		listView.AddColumn("ʱ��", 150);
		listView.AddColumn("����", 90);
		listView.AddColumn("�����Ѷ�", 60);
		listView.AddColumn("��߷�", 60);
		listView.AddColumn("�Ƿ���", 60);
	}

	//�������
	UpdateListViewAndStatic();

	//��ǩҳ
	tabControl.LinkControl(m_hWnd, IDC_TAB);
	tabControl.SetRectAsParent();
	tabControl.AddTabItem("����", { &vecListView[0],&vecStaticHighScore[0],&vecStaticWinNum[0] });
	tabControl.AddTabItem("�м�", { &vecListView[1],&vecStaticHighScore[1],&vecStaticWinNum[1] });
	tabControl.AddTabItem("�߼�", { &vecListView[2],&vecStaticHighScore[2],&vecStaticWinNum[2] });
	tabControl.SetCurSel(0);

	return 0;
}

int DialogHighScore::GetSuit(int page)
{
	switch (page)
	{
	case 0:return 1;
	case 1:return 2;
	case 2:return 4;
	}
}

LRESULT DialogHighScore::OnReplay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//ȡ��ѡ����
	int page = tabControl.GetCurSel();
	auto selection = vecListView[page].GetCurSel();
	if (selection.size() != 1)
	{
		MessageBox("ֻ��ѡ��һ�", "��ʾ", MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	//���÷���ֵ
	ReturnType* pret = new ReturnType;
	pret->isRandom = false;
	pret->suit = GetSuit(page);
	pret->seed=stoul(vecListView[page].GetItem(selection.front(),1));
	pret->calc = 0;
	pret->solved = (vecListView[page].GetItem(selection.front(), 4)) == "�ѽ��" ? true : false;

	//�򸸴��ڷ�������Ϸ��Ϣ��2=������
	::PostMessage(hParent, WM_COMMAND, MAKELONG(ID_NEW_GAME, 2), (LPARAM)pret);
	EndDialog(0);

	return 0;
}

LRESULT DialogHighScore::OnEvaluate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//ȡ��ѡ����
	int page = tabControl.GetCurSel();
	auto selection = vecListView[page].GetCurSel();
	if (selection.empty())
	{
		MessageBox("û��ѡ���κμ�¼��", "��ʾ", MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	//������������
	vector<ReturnType> input;
	for (auto index : selection)
	{
		ReturnType temp;
		temp.suit = GetSuit(page);
		temp.seed=stoul(vecListView[page].GetItem(index, 1));
		temp.reserved = index;

		input.push_back(temp);
	}

	//����
	DialogEvaluate dialogEvaluate(GetSuit(page), input);
	dialogEvaluate.DoModal();

	//ȡ���������
	auto ret = dialogEvaluate.ret;
	for (auto& pr : *ret)
	{
		//���¼�¼
		config.UpdateRecord(pr.second.suit, pr.second.seed, 0, false, pr.second.calc,!pr.second.solved);

		//�����б��
		vecListView[page].UpdateItem(pr.second.reserved, config.seedMap[page][pr.second.seed]->ToVecString());
	}

	return 0;
}

LRESULT DialogHighScore::OnDeleteAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IDYES == MessageBox("�Ƿ�������м�¼��", "ѯ��", MB_YESNO | MB_ICONQUESTION))
	{
		config.ClearRecord();
		UpdateListViewAndStatic();
	}

	return 0;
}