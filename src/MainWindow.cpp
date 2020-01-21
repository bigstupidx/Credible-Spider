#include "MainWindow.h"

#include "Configuration.h"
#include "DialogChooseLevel.h"
#include "DialogHighScore.h"
#include "DialogAuto.h"
#include "DialogAbout.h"

#include "RectShadow.h"
#pragma comment(lib,"winmm.lib")

#include <thread>

using namespace std;

extern Configuration config;
const string savFileName = "credible spider.sav";


LRESULT MainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//���ز˵�
	HMENU hmenu = LoadMenu(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDR_MENU_MAIN));
	SetMenu(hmenu);

	//����ͼ��
	HICON hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON));
	SetIcon(hIcon, FALSE);

	//���ñ���
	imgBackground = new TImage(GetModuleHandle(NULL), IDB_BACKGROUND);
	hdcBackground = CreateCompatibleDC(NULL);

	//rectShadow = nullptr;

	//��ʼ��manager
	manager = make_shared<Manager>();
	manager->SetSoundId(IDR_WAVE_TIP, IDR_WAVE_NOTIP, IDR_WAVE_SUCCESS, IDR_WAVE_DEAL);
	manager->SetGUIProperty(m_hWnd, &rcClient, IDB_CARDEMPTY, IDB_CARDBACK, IDB_CARD1, IDB_CARDMASK);

	//��������
	config.ReadFromFile(savFileName);

	CheckMenuItem(GetMenu(), ID_ENABLE_ANIMATION, config.enableAnimation ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetMenu(), ID_ENABLE_SOUND, config.enableSound ? MF_CHECKED : MF_UNCHECKED);

	RefreshMenu();

	PostMessage(WM_COMMAND, MAKELONG(ID_NEW_GAME, 0), 0);
	return 0;
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	static bool ask = true;
	int answer;
	if (ask)
		answer = MessageBox(TEXT("�رո���Ϸ֮ǰ�Ƿ���Ҫ���棿"), TEXT("Credible Spider"), MB_YESNOCANCEL | MB_ICONQUESTION);
	else
		answer = IDNO;
	switch (answer)
	{
	case IDYES:
	case IDNO:
	{
		//���manager���ڶ���
		if (manager->bOnThread)
		{
			//����ֹͣ��ǣ�����ִ�еĶ�����鵽��Ǻ�������������˳�
			manager->bStopThread = true;

			//����Ϊ��ѯ�ʣ������´ν��뽫��ѯ��
			ask = false;

			//�ٴη���WM_CLOSE��Ϣ
			//ʹ�����ַ�ʽ����Ϊ���������UpdataWindow������˴�����������ִ�еĶ���������
			//���ԣ��˴������������ر���Ϣ�ٴμ�����У�ȷ���ر���Ϣʼ�մ��ڶ�����
			//�������˴�����ε������⶯���Ƿ����
			PostMessage(WM_CLOSE);
			break;
		}
		else
		{
			if (config.SaveToFile(savFileName) == false)
				MessageBox(("���������ļ� " + savFileName + " ʧ��.").c_str(), "����", MB_OK | MB_ICONERROR);
			DestroyWindow();
		}
	}
	case IDCANCEL:
		break;
	}
	return 0;
}

LRESULT MainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	delete imgBackground;

	//delete rectShadow;

	ReleaseDC(hdcMem);

	DeleteDC(hdcBackground);
	PostQuitMessage(0);
	return 0;
}



void MainWindow::Draw(HDC hdc, const RECT& rect)
{
	//���ϱ���
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcBackground, 0, 0, SRCCOPY);


	//������Ӱ
	//rectShadow->Draw(hdc);

	//����TipBox
	tipBox.Draw(hdc, manager);

	//�����ƾ�
	manager->Draw(hdc, rect);
}

LRESULT MainWindow::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//HDC hdc;
	//RECT rect;
	//GetClientRect(&rect);
	//PAINTSTRUCT ps;
	//hdc = BeginPaint(&ps);

	//imgBackground->Fill(hdc, rect);

	//EndPaint(&ps);
	return false;
}

LRESULT MainWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#ifdef _DEBUG
	//static DWORD prev = 0, now;
	//now = GetTickCount();
	//OutputDebugString((to_string(1000.0/(now - prev))+"\n").c_str());
	//prev = now;
#endif

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);

	if (doubleBuffer)
	{
		Draw(hdcMem, rcClient);

		BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
	}
	else
		Draw(ps.hdc, rcClient);

	EndPaint(&ps);
	return 0;
}


LRESULT MainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//��ΪWM_SIZE���ֶ����ͣ�����rcClient����ȡ��wParam�е�ֵ
	GetClientRect(&rcClient);

	tipBox.OnSize(rcClient, border);

	HDC hdc = GetDC();

	ReleaseDC(hdcMem);
	hdcMem = CreateCompatibleDC(hdc);

	//����hdcMem��С
	HBITMAP hBitmapMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
	SelectObject(hdcMem, hBitmapMem);
	DeleteObject(hBitmapMem);

	//������Ӱ
	//delete rectShadow;
	//rectShadow = new RectShadow(hdc, m_hWnd, rectTipBox, 10, -45, 5.0);

	if (imgBackground)
	{
		ReleaseDC(hdcBackground);
		hdcBackground = CreateCompatibleDC(hdc);

		//����hdcBackground��С
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
		SelectObject(hdcBackground, hBitmap);
		DeleteObject(hBitmap);

		imgBackground->Fill(hdcBackground, rcClient);

	}
	ReleaseDC(hdc);

	if (manager)
		manager->OnSize(rcClient);

	return 0;
}

LRESULT MainWindow::OnReNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IDYES == MessageBox("�Ƿ����¿�ʼ������Ϸ��", "ѯ��", MB_YESNO))
	{
		EnableAllInput(false);

		//ϴ��
		manager->Command("new " + std::to_string(manager->GetPokerSuitNum()) + " " + std::to_string(manager->GetPokerSeed()));

		EnableAllInput(true);

		RefreshMenu();
	}
	return 0;
}

LRESULT MainWindow::OnNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	static bool bOpenDialog = true;
	static ReturnType ret = { 0 };

	if (wNotifyCode==2)
	{
		ReturnType* pret = (ReturnType*)hWndCtl;
		bOpenDialog = false;
		ret = *pret;
		delete pret;
	}

	if (bOpenDialog)
	{
		DialogChooseLevel dialogChooseLevel;
		dialogChooseLevel.DoModal();
		if (dialogChooseLevel.ret)
		{
			ret = *dialogChooseLevel.ret;
		}
		else
			ret.suit = 0;
	}

	if (ret.suit != 0)
	{
		if (manager->bOnThread)
		{
			manager->bStopThread = true;
			bOpenDialog = false;
			PostMessage(WM_COMMAND, MAKELONG(ID_NEW_GAME, 0), 0);
			return 0;
		}

		manager->bStopThread = false;
		bOpenDialog = true;
		auto fun = [&](int suit)
		{
			EnableAllInput(false);


			if (ret.isRandom)
				//�������Ϸ
				manager->Command("newrandom " + std::to_string(suit));
			else
				manager->Command("new " + std::to_string(suit) + " " + std::to_string(ret.seed));

			//������;���˳�
			if (manager->bStopThread)
				return;

			EnableAllInput(true);

			//
			config.UpdateRecord(manager->GetPokerSuitNum(),manager->GetPokerSeed(),manager->GetPokerScore(),false,ret.calc);

			RefreshMenu();
		};
		thread t(fun, ret.suit);
		t.detach();
	}

	return 0;
}

void MainWindow::RefreshMenu()
{
	if (manager->HasPoker())
	{
		EnableMenuItem(GetMenu(), ID_RENEW_GAME, MF_ENABLED);//���¿�ʼ����Ϸ
	}
	else
	{
		EnableMenuItem(GetMenu(), ID_RENEW_GAME, MF_GRAYED);
	}

	if (manager->HasPoker() && !manager->GetIsWon())
	{
		EnableMenuItem(GetMenu(), ID_AUTO, MF_ENABLED);//�����Զ�����
		EnableMenuItem(GetMenu(), ID_SHOWMOVE, MF_ENABLED);//������ʾ���ƶ�
		EnableMenuItem(GetMenu(), ID_SAVE, MF_ENABLED);//
	}
	else
	{
		EnableMenuItem(GetMenu(), ID_AUTO, MF_GRAYED);
		EnableMenuItem(GetMenu(), ID_SHOWMOVE, MF_GRAYED);
		EnableMenuItem(GetMenu(), ID_SAVE, MF_GRAYED);//
	}

	//ˢ�� ���� ����
	if (manager->CanRedo())
		EnableMenuItem(GetMenu(), ID_REDO, MF_ENABLED);//���ó���
	else
		EnableMenuItem(GetMenu(), ID_REDO, MF_GRAYED);

	//ˢ�� ���� ����
	if (manager->HasPoker() && !manager->PokerCornerIsEmpty())
	{
		EnableMenuItem(GetMenu(), ID_RELEASE, MF_ENABLED);//���÷���
		EnableMenuItem(GetMenu(), ID_RELEASE2, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(GetMenu(), ID_RELEASE, MF_GRAYED);
		EnableMenuItem(GetMenu(), ID_RELEASE2, MF_GRAYED);
	}

	//ˢ��
	DrawMenuBar();
}

void MainWindow::EnableAllInput(bool enable)
{
	static vector<vector<UINT>> origin;
	if (enable == false)
	{
		//������������󻯿�
		LONG style = GetWindowLong(GWL_STYLE);
		style ^= WS_THICKFRAME | WS_MAXIMIZEBOX;
		SetWindowLong(GWL_STYLE, style);

		//�������в˵������
		origin.clear();

		HMENU hMenu = GetMenu();
		for (int i = 0; i < GetMenuItemCount(hMenu); ++i)
		{
			vector<UINT> temp;
			HMENU hSubMenu = GetSubMenu(hMenu, i);

			int id = GetMenuItemID(hMenu, i);
			UINT state = GetMenuState(hMenu, id, MF_BYCOMMAND);
			temp.push_back(state);
			EnableMenuItem(hMenu, id, MF_GRAYED);
			for (int j = 0; j < GetMenuItemCount(hSubMenu); ++j)
			{
				int id = GetMenuItemID(hSubMenu, j);

				UINT state = GetMenuState(hMenu, id, MF_BYCOMMAND);
				temp.push_back(state);
				EnableMenuItem(hMenu, id, MF_GRAYED);
			}
			origin.push_back(temp);
		}
		DrawMenuBar();
	}
	else
	{
		//������������󻯿�
		LONG style = GetWindowLong(GWL_STYLE);
		style ^= WS_THICKFRAME | WS_MAXIMIZEBOX;
		SetWindowLong(GWL_STYLE, style);

		//�ָ����в˵���
		HMENU hMenu = GetMenu();
		for (int i = 0; i < GetMenuItemCount(hMenu); ++i)
		{
			HMENU hSubMenu = GetSubMenu(hMenu, i);

			int id = GetMenuItemID(hMenu, i);
			EnableMenuItem(hMenu, id, origin[i][0]);
			for (int j = 0; j < GetMenuItemCount(hSubMenu); ++j)
			{
				int id = GetMenuItemID(hSubMenu, j);

				EnableMenuItem(hMenu, id, origin[i][j + 1]);
			}
		}
		origin.clear();
		DrawMenuBar();
	}
}

LRESULT MainWindow::OnRelease(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (manager->Command("r"))
		RefreshMenu();
	else
	{
		MessageBox("�п�λ���ܷ��ơ�", "��ʾ", MB_OK);
	}
	return 0;
}

LRESULT MainWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);

	if (!manager->HasPoker())
		return 0;

	if (manager->bOnThread)
		return 0;

	if (manager->OnLButtonDown(ptPos))
	{
		if (config.enableSound)
			PlaySound((LPCSTR)IDR_WAVE_PICKUP, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	}
	else
	{
		if (PtInRect(&tipBox.GetRect(), ptPos))
			manager->ShowOneHint();
		else
			if (manager->PtInRelease(ptPos))
				OnRelease(0, 0, 0, bHandled);
	}
	return 0;
}

LRESULT MainWindow::OnShowMove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	manager->ShowOneHint();
	return 0;
}

LRESULT MainWindow::OnHighScore(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DialogHighScore dialogHighScore(m_hWnd);
	dialogHighScore.DoModal();
	return 0;
}

LRESULT MainWindow::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (manager->bOnThread)
		return 0;

	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);


	if (manager->OnLButtonUp(ptPos))
	{
		if (config.enableSound)
			PlaySound((LPCSTR)IDR_WAVE_PUTDOWN, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

		config.UpdateRecord(manager->GetPokerSuitNum(), manager->GetPokerSeed(), manager->GetPokerScore());

		if (manager->GetIsWon())
		{
			config.UpdateRecord(manager->GetPokerSuitNum(), manager->GetPokerSeed(), manager->GetPokerScore(),true);
			manager->Win();
		}

		RefreshMenu();

	}
	return 0;
}

LRESULT MainWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (manager->bOnThread)
		return 0;

	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);


	manager->OnMouseMove(ptPos);

	return 0;
}


LRESULT MainWindow::OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	manager->Command("redo");

	RefreshMenu();
	return 0;
}

LRESULT MainWindow::OnAuto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DialogAuto dialogAuto(manager);
	dialogAuto.DoModal();
	auto ret = dialogAuto.ret;

	config.UpdateRecord(manager->GetPokerSuitNum(), manager->GetPokerSeed(), manager->GetPokerScore(),ret->solved,ret->calc);

	if (manager->GetIsWon())
	{
		manager->Win();
	}

	RefreshMenu();

	return 0;
}

LRESULT MainWindow::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DialogAbout dialogAbout;
	dialogAbout.DoModal();

	return 0;
}

LRESULT MainWindow::OnQuit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SendMessage(WM_CLOSE);

	return 0;
}

LRESULT MainWindow::OnSetOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	UINT state = GetMenuState(GetMenu(), wID, MF_BYCOMMAND);
	if (state & MF_CHECKED)
		state = MF_UNCHECKED;
	else
		state = MF_CHECKED;
	CheckMenuItem(GetMenu(), wID, state);
	switch (wID)
	{
	case ID_ENABLE_ANIMATION:
		config.enableAnimation = state & MF_CHECKED;
		break;
	case ID_ENABLE_SOUND:
		config.enableSound = state & MF_CHECKED;
		break;
	}
	return 0;
}