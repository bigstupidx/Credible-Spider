#include "MainWindow.h"

#include "Configuration.h"
#include "DialogChooseLevel.h"
#include "DialogAuto.h"
#include "DialogAbout.h"

#pragma comment(lib,"winmm.lib")

#include <thread>

using namespace std;

extern Configuration config;
const string savFileName = "credible spider.sav";

void DrawTextAdvance(HDC hdc, const TCHAR text[], RECT *rect, long FontSize, int FontWeight, COLORREF color, const TCHAR FontName[], UINT format, int cEscapement = 0, int cOrientation = 0)
{
	long lfHeight = -MulDiv(FontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	HFONT hf = CreateFont(lfHeight, 0, cEscapement, cOrientation, FontWeight, 0, 0, 0, 0, 0, 0, 0, 0, FontName);

	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, hf);
	COLORREF crPrev = SetTextColor(hdc, color);
	DrawText(hdc, text, -1, rect, format);
	SetTextColor(hdc, crPrev);
	DeleteObject(hf);
}

void DrawTextCenter(HDC hdc, const TCHAR text[], const RECT &rect, long FontSize, int FontWeight, COLORREF color, const TCHAR FontName[], UINT format)
{
	RECT rectText = rect;
	DrawTextAdvance(hdc, text, &rectText, FontSize, FontWeight, color, FontName, format | DT_CALCRECT);

	int width = rectText.right - rectText.left;
	int height = rectText.bottom - rectText.top;

	rectText.left = (rect.right + rect.left) / 2 - width / 2;
	rectText.top = (rect.bottom + rect.top) / 2 - height / 2;
	rectText.right = rectText.left + width;
	rectText.bottom = rectText.top + height;

	DrawTextAdvance(hdc, text, &rectText, FontSize, FontWeight, color, FontName, format);
}


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

	//��ʾ��
	hBrushTipBox = CreateSolidBrush(crTipBox);

	//��ʼ��manager
	manager = make_shared<Manager>();
	manager->SetSoundId(IDR_WAVE_TIP, IDR_WAVE_NOTIP,IDR_WAVE_SUCCESS,IDR_WAVE_DEAL);
	manager->SetGUIProperty(m_hWnd, IDB_CARDEMPTY, IDB_CARDBACK, IDB_CARD1,IDB_CARDMASK);

	if (config.LoadFromFile(savFileName)==false)
	{
		config.enableAnimation = false;
		config.enableSound = true;
	}  
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
			//�˴������������ر���Ϣ�ٴμ�����У�ȷ���ر���Ϣʼ�մ��ڶ�����
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

	DeleteObject(hBrushTipBox);
	PostQuitMessage(0);
	return 0;
}

void MainWindow::Draw(HDC hdc, const RECT &rect)
{
	//Background
	imgBackground->Fill(hdc, rect);

	//TipBox
	if (manager->HasPoker())
	{
		textTipBox = "������" + std::to_string(manager->GetPokerScore()) + "\r\n";
		textTipBox += "������" + std::to_string(manager->GetPokerOperation());
	}

	SelectObject(hdc, hBrushTipBox);
	Rectangle(hdc, rectTipBox.left, rectTipBox.top, rectTipBox.right, rectTipBox.bottom);

	DrawTextCenter(hdc, textTipBox.c_str(), rectTipBox, 12, 400, RGB(255, 255, 255), TEXT("����"), DT_LEFT);

	//
	manager->Draw(hdc,rect);
}

LRESULT MainWindow::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return false;
}

LRESULT MainWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	hdc = BeginPaint(&ps);
	GetClientRect(&rect);

	if (doubleBuffer)
	{
		HDC hDCMem;
		HBITMAP hBitmap;
		hDCMem = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
		SelectObject(hDCMem, hBitmap);

		Draw(hDCMem, rect);

		BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDCMem, 0, 0, SRCCOPY);
		DeleteObject(hBitmap);
		DeleteDC(hDCMem);
	}
	else
		Draw(ps.hdc, rect);

	EndPaint(&ps);
	return 0;
}


LRESULT MainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetClientRect(&rect);

	//ˢ����ʾ��λ��
	rectTipBox.left = (rect.right - TIPBOX_WIDTH) / 2;
	rectTipBox.right = rectTipBox.left + TIPBOX_WIDTH;
	rectTipBox.bottom = rect.bottom - border;
	rectTipBox.top = rectTipBox.bottom - TIPBOX_HEIGHT;

	if (manager)
		manager->OnSize(rect);

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
	static int suit=0;
	static bool isRandom = true;
	static uint32_t seed = 0;
	if (bOpenDialog)
	{
		DialogChooseLevel dialogChooseLevel;
		dialogChooseLevel.DoModal();
		if (dialogChooseLevel.ret)
		{
			isRandom = dialogChooseLevel.ret->isRandom;
			suit = dialogChooseLevel.ret->suit;
			seed = dialogChooseLevel.ret->seed;
		}
		else
			suit = 0;
	}
	if (suit != 0)
	{
		if (manager->bOnThread)
		{
			manager->bStopThread = true;
			bOpenDialog = false;
			PostMessage(WM_COMMAND, MAKELONG(ID_NEW_GAME,0 ), 0);
			return 0;
		}

		manager->bStopThread = false;
		bOpenDialog = true;
		auto fun = [&](int suit)
		{
			EnableAllInput(false);

			if (isRandom)
				//�������Ϸ
				manager->Command("newrandom " + std::to_string(suit));
			else
				manager->Command("new " + std::to_string(suit)+" "+std::to_string(seed));

			//������;���˳�
			if (manager->bStopThread)
				return;

			EnableAllInput(true);

			RefreshMenu();
		};
		thread t(fun, suit);
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

				EnableMenuItem(hMenu, id, origin[i][j+1]);
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
		if (PtInRect(&rectTipBox, ptPos))
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

		if (manager->GetIsWon())
			manager->Win();

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

	if (manager->GetIsWon())
		manager->Win();

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