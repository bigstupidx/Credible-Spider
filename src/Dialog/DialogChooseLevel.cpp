#include "DialogChooseLevel.h"

#include "Manager.h"
#include "TStatic.h"
#include <string>
#include <thread>

using namespace std;

LRESULT DialogChooseLevel::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ret = new DialogChooseLevelReturnType{ true,0,0 };

	editSeed.LinkControl(m_hWnd, IDC_EDIT_SEED);
	editSeed.SetEnable(false);

	cbCanBeSolved.LinkControl(m_hWnd, IDC_CHECK_CANBESOLVED);

	rbGroupLevel.LinkControl(m_hWnd, { IDC_RB_LEVEL1,IDC_RB_LEVEL2, IDC_RB_LEVEL4 }, { 1,2,4 });
	rbGroupLevel.SetChecked(IDC_RB_LEVEL1);

	rbGroupSeed.LinkControl(m_hWnd, { IDC_RADIO_RANDOMSEED,IDC_RADIO_INPUTSEED }, { 1,0 });
	rbGroupSeed.SetChecked(IDC_RADIO_RANDOMSEED);

	btnOK.LinkControl(m_hWnd, IDOK);

	btnCancel.LinkControl(m_hWnd, IDCANCEL);

	//HINSTANCE hInst = GetModuleHandle(NULL);
	//HWND hStatic = CreateWindow("static", NULL, WS_CHILD | WS_VISIBLE, 0, 0, 100, 100,
	//	m_hWnd, 0, hInst, 0);
	//::SetWindowText(hStatic, "aaa");

	CenterWindow();
	return TRUE;    // let the system set the focus
}

LRESULT DialogChooseLevel::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsManagerOnThread())
	{
		StopManagerThread();
		PostMessage(WM_CLOSE);
		return 0;
		
	}

	delete ret;
	EndDialog(0);
	return 0;
}

LRESULT DialogChooseLevel::OnPropertyChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	WORD& id = wID;
	ret->suit = rbGroupLevel.ReceiveCommand(id);
	ret->isRandom = rbGroupSeed.ReceiveCommand(id);

	if (ret->suit != 4 && ret->isRandom)
	{
		cbCanBeSolved.SetEnable(true);
	}
	else
	{
		cbCanBeSolved.SetEnable(false);
		cbCanBeSolved.SetChecked(false);
	}


	editSeed.SetEnable(ret->isRandom == false);

	return 0;
}

void SetEnable(vector<TControl*> vec, bool enable)
{
	static unordered_map<TControl*, bool> states;

	if (enable == false)
	{
		states.clear();
		for (auto& p : vec)
		{
			states[p] = p->GetEnable();
			p->SetEnable(false);
		}
	}
	else
	{
		for (auto& p : vec)
			p->SetEnable(states[p]);
		states.clear();
	}
}

bool DialogChooseLevel::IsManagerOnThread()
{
	for (auto& manager : vecManager)
		if (manager && manager->bOnThread)
		{
			return true;
		}
	return false;
}

void DialogChooseLevel::StopManagerThread()
{
	for (auto& manager : vecManager)
		if (manager->bOnThread)
		{
			manager->bStopThread = true;
		}
}

LRESULT DialogChooseLevel::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int id = LOWORD(wParam);

	ret->suit = rbGroupLevel.GetValue();
	ret->isRandom = rbGroupSeed.GetValue();

	switch (id)
	{
	case IDOK:
	{
		if (IsManagerOnThread())
		{
			StopManagerThread();
			return 0;
		}

		if (ret->isRandom == false)
		{
			try
			{
				ret->seed = std::stoul(editSeed.GetText());
			}
			catch (...)
			{
				MessageBox(("无效的种子。范围：0-" + std::to_string(UINT_MAX)).c_str(), "错误", MB_OK | MB_ICONERROR);
				::SetFocus(editSeed.GetHWND());
				break;
			}
		}

		if (cbCanBeSolved.GetChecked())
		{
			auto fun = [&]()
			{
				//设置界面为搜索状态
				vector<TControl*> vecCtrl{ &rbGroupLevel,&rbGroupSeed,&cbCanBeSolved,&editSeed };
				SetEnable(vecCtrl, false);
				btnOK.SetText("停止");
				RECT rcNewClient;
				GetClientRect(&rcNewClient);
				RECT rcStatic = { rcNewClient.left,rcNewClient.bottom - 100,rcNewClient.right,rcNewClient.bottom };
				rcNewClient.bottom += 0;
				//MoveWindow(&rcNewClient);


				auto search = [&](std::shared_ptr<Manager> &manager)
				{
					while (1)
					{
						if (manager->Command("auto"))
						{
							ret->isRandom = false;
							ret->seed = manager->GetPoker()->seed;
							break;
						}

						if (manager->bStopThread)
							break;
					}
				};

				int threadNum = 4;
				vecManager = vector<shared_ptr<Manager>>(threadNum, nullptr);
				for (auto& manager : vecManager)
				{
					manager = make_shared<Manager>(ret->suit);
					manager->SetTextOutputHWND(m_hWnd);
					manager->bOnThread = true;

					thread t(search,manager);
					t.detach();
				}

				shared_ptr<Manager> resultManager;
				while (IsManagerOnThread())
				{
					this_thread::sleep_for(10ms);
					bool found = false;
					for (auto &manager:vecManager)
						if (manager->autoSolveResult.success == true)
						{
							resultManager = manager;
							found = true;
							break;
						}
					if (found)
						break;
				}

				StopManagerThread();

				SetEnable(vecCtrl, true);
				btnOK.SetText("确定");

				if (resultManager)
				{
					stringstream ss;
					ss << "找到一组可解题目：" << endl << endl;
					ss << "花色=" << resultManager->autoSolveResult.suit << endl;
					ss << "seed=" << resultManager->autoSolveResult.seed << endl;
					ss << "尝试次数=" << resultManager->autoSolveResult.calc << endl;
					ss << "难度=" << -10000.0 / resultManager->autoSolveResult.calc + 100 << endl;

					MessageBox(ss.str().c_str(), "求解结果", MB_OK | MB_ICONINFORMATION);
					EndDialog((int)ret);
				}
			};

			thread t(fun);
			t.detach();
		}
		else
			EndDialog((int)ret);
		break;
	}
	case IDCANCEL:

		SendMessage(WM_CLOSE);
		break;
	}

	return 0;
}