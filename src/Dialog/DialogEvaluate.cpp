#include "DialogEvaluate.h"

#include <string>
#include <thread>

#include <sstream>
#include <queue>

using namespace std;

LRESULT DialogEvaluate::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	btnCancel.LinkControl(m_hWnd, IDCANCEL);

	SetWindowText("��������...");

	//ȡ��CPU�߼�������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	threadNum = sysInfo.dwNumberOfProcessors;
	threadNum = min(threadNum / 2, input.size());

	int fontHeight = 18;//�ָ�
	int border = 10;//���
	int dHeight = (threadNum + 1) * fontHeight;//�����Ӹ߶�
	RECT rectClient;
	GetClientRect(&rectClient);

	RECT rect;
	GetWindowRect(&rect);
	int captionHeight = (rect.bottom - rect.top) - rectClient.bottom;
	rect.bottom += dHeight;//���Ӹ߶�
	rect.right += 100;//���ӿ��
	MoveWindow(&rect);

	//�Ի����ܿ��
	int width = rect.right - rect.left;

	//���ð�ťλ��
	RECT rectBtn;
	::GetWindowRect(btnCancel.GetHWND(), &rectBtn);
	int x = rectBtn.left - rect.left;
	int y = rectBtn.top - rect.top - captionHeight;
	int w = rectBtn.right - rectBtn.left;
	int h = rectBtn.bottom - rectBtn.top;
	x = (width - w) / 2;//��ť����
	y += dHeight + border;//��ť�����ƶ�
	::MoveWindow(btnCancel.GetHWND(), x, y, w, h, 1);

	//�߳������ı�
	staticMemo.Create(m_hWnd, { border,border,width,fontHeight });
	staticMemo.SetText("�߳�������" + to_string(threadNum));
	staticMemo.SetDefaultGuiFont();

	//�����߳��ı�
	for (int i = 0; i < threadNum; ++i)
	{
		vecStatic.emplace_back();
		vecStatic.back().Create(m_hWnd, { border,border + fontHeight * (i + 1),width,fontHeight });
		vecStatic.back().SetDefaultGuiFont();
	}

	CenterWindow();

	//��ʼ������ֵ
	ret = make_shared<unordered_map<uint32_t, ReturnType>>();

	for (auto& returnType : input)
	{
		(*ret).insert({ returnType.seed,returnType });
	}

	PostMessage(WM_COMMAND, MAKELONG(IDOK, 0));

	return 0;
}

LRESULT DialogEvaluate::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//֪ͨmanagerֹͣ
	for (auto& manager : managers)
		manager->bStopThread = true;

	if (onThread == false)
		EndDialog(0);

	return 0;
}

LRESULT DialogEvaluate::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	int id = LOWORD(wParam);

	switch (id)
	{
	case IDOK:
	{
		auto mainThread = [&]()
		{
			onThread = true;

			//�Ѵ�����
			int solvedNum = 0;

			mutex m;

			//������������
			queue<ReturnType> q;
			for (auto& returnType : input)
				q.push(returnType);

			//�����߳�
			auto EvaluateThread = [&](int index)
			{
				while (1)
				{
					m.lock();

					//�����ѿգ��˳�
					if (q.empty())
					{
						m.unlock();
						break;
					}

					//ȡ��һ����������
					auto raw = q.front();
					q.pop();

					//�½�manager������managers
					shared_ptr<Manager> manager = make_shared<Manager>(suit, raw.seed);
					managers.insert(manager);

					m.unlock();

					//���ÿؼ�
					vecStatic[index].SetText("�߳�" + to_string(index) + " ����=" + to_string(raw.seed));
					manager->SetTextOutputHWND(vecStatic[index].GetHWND());

					//��ʼ����
					if (manager->Command("auto"))
						solvedNum++;

					m.lock();

					//������
					(*ret)[raw.seed].solved = manager->autoSolveResult.success;
					(*ret)[raw.seed].calc = manager->autoSolveResult.calc;

					m.unlock();

					//���ÿؼ�
					vecStatic[index].SetText("�߳�" + to_string(index) + " ���˳���");
				}
			};

			//�����߳�
			vector<thread> vecThread;
			for (int index = 0; index < threadNum; ++index)
			{
				vecThread.push_back(thread(EvaluateThread, index));
			}

			//�����߳�
			for_each(vecThread.begin(), vecThread.end(), [](auto& t) {t.join(); });

			//����
			stringstream ss;
			ss << "�����ѽ�����" << endl << endl;
			ss << "���������" << solvedNum << "/" << ret->size();

			MessageBox(ss.str().c_str(), "��������", MB_OK | MB_ICONINFORMATION);

			onThread = false;

			SendMessage(WM_CLOSE);
		};

		thread t(mainThread);
		t.detach();
		break;
	}
	case IDCANCEL:
	{
		SendMessage(WM_CLOSE);
		break;
	}
	}

	return 0;
}