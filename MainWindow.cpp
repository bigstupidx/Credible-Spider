#include "MainWindow.h"

#include "DialogChooseLevel.h"
#include "DialogAuto.h"
#include "DialogAbout.h"

#pragma comment(lib,"winmm.lib")

#include <thread>
using namespace std;

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
	//加载菜单
	HMENU hmenu = LoadMenu(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDR_MENU_MAIN));
	SetMenu(hmenu);

	//加载图标
	HICON hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON));
	SetIcon(hIcon, FALSE);

	imgBackground = new TImage(GetModuleHandle(NULL), IDB_BACKGROUND);

	manager = make_shared<Manager>();
	manager->SetSoundId(IDR_WAVE_TIP, IDR_WAVE_NOTIP,IDR_WAVE_SUCCESS);
	manager->SetGUIProperty(m_hWnd, IDB_CARDEMPTY, IDB_CARDBACK, IDB_CARD1);

	hBrushTipBox = CreateSolidBrush(crTipBox);


	bOnDrag = false;

	RefreshMenu();

	PostMessage(WM_COMMAND, MAKELONG(ID_NEW_GAME, 0), 0);
	return 0;
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	static bool ask = true;
	int answer;
	if (ask)
		answer = MessageBox(TEXT("关闭该游戏之前是否需要保存？"), TEXT("Credible Spider"), MB_YESNOCANCEL | MB_ICONQUESTION);
	else
		answer = IDNO;
	switch (answer)
	{
	case IDYES:
	case IDNO:
	{
		//如果manager正在动画
		if (manager->bOnThread)
		{
			//设置停止标记，正在执行的动画检查到标记后做好清理进行退出
			manager->bStopThread = true;

			//设置为不询问，这样下次进入将不询问
			ask = false;

			//再次发送WM_CLOSE消息
			//使用这种方式是因为动画会调用UpdataWindow，如果此处阻塞，正在执行的动画将卡死
			//此处不阻塞，将关闭消息再次加入队列，确保关闭消息始终处于队列中
			//这样，此处将多次到达，并检测动画是否结束
			PostMessage(WM_CLOSE);
			break;
		}
		else
			DestroyWindow();
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
	if (manager->GetPoker())
	{
		textTipBox = "分数：" + std::to_string(manager->GetPoker()->score) + "\r\n";
		textTipBox += "操作：" + std::to_string(manager->GetPoker()->operation);
	}

	SelectObject(hdc, hBrushTipBox);
	Rectangle(hdc, rectTipBox.left, rectTipBox.top, rectTipBox.right, rectTipBox.bottom);

	DrawTextCenter(hdc, textTipBox.c_str(), rectTipBox, 12, 400, RGB(255, 255, 255), TEXT("宋体"), DT_LEFT);

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

	//刷新提示框位置
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
	if (IDYES == MessageBox("是否重新开始本局游戏？", "询问", MB_YESNO))
	{
		EnableAllInput(false);

		//洗牌
		manager->Command("new " + std::to_string(manager->GetPoker()->suitNum) + " " + std::to_string(manager->GetPoker()->seed));

		EnableAllInput(true);

		RefreshMenu();
	}
	return 0;
}

LRESULT MainWindow::OnNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	static bool bOpenDialog = true;
	static int suit=0;
	if (bOpenDialog)
	{
		DialogChooseLevel dialogChooseLevel;
		suit = dialogChooseLevel.DoModal();
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
			//随机新游戏
			manager->Command("newrandom " + std::to_string(suit));

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
	if (manager->GetPoker())
	{
		//重新开始新游戏选项可见
		EnableMenuItem(GetMenu(), ID_RENEW_GAME, MF_ENABLED);

		EnableMenuItem(GetMenu(), ID_SHOWMOVE, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(GetMenu(), ID_RENEW_GAME, MF_GRAYED);

		EnableMenuItem(GetMenu(), ID_SHOWMOVE, MF_GRAYED);
	}

	if (manager->GetPoker() && !manager->GetIsWon())
		EnableMenuItem(GetMenu(), ID_AUTO, MF_ENABLED);
	else
		EnableMenuItem(GetMenu(), ID_AUTO, MF_GRAYED);

	//刷新 撤销 命令
	if (manager->CanRedo())
		EnableMenuItem(GetMenu(), ID_REDO, MF_ENABLED);
	else
		EnableMenuItem(GetMenu(), ID_REDO, MF_GRAYED);

	//刷新 发牌 命令
	if (manager->GetPoker() && !manager->GetPoker()->corner.empty())
	{
		EnableMenuItem(GetMenu(), ID_RELEASE, MF_ENABLED);
		EnableMenuItem(GetMenu(), ID_RELEASE2, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(GetMenu(), ID_RELEASE, MF_GRAYED);
		EnableMenuItem(GetMenu(), ID_RELEASE2, MF_GRAYED);
	}

	DrawMenuBar();
}

void MainWindow::EnableAllInput(bool enable)
{
	static vector<vector<UINT>> origin;
	if (enable == false)
	{
		//禁用缩放与最大化框
		LONG style = GetWindowLong(GWL_STYLE);
		style ^= WS_THICKFRAME | WS_MAXIMIZEBOX;
		SetWindowLong(GWL_STYLE, style);

		//保存所有菜单项并禁用
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
		//启用缩放与最大化框
		LONG style = GetWindowLong(GWL_STYLE);
		style ^= WS_THICKFRAME | WS_MAXIMIZEBOX;
		SetWindowLong(GWL_STYLE, style);

		//恢复所有菜单项
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
		MessageBox("有空位不能发牌。", "提示", MB_OK);
	}
	return 0;
}

LRESULT MainWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);

	if (!manager->GetPoker())
		return 0;

	if (manager->bOnThread)
		return 0;

	if (manager->OnLButtonDown(ptPos))
		PlaySound((LPCSTR)IDR_WAVE_PICKUP, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
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
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);

	if (manager->bOnThread)
		return 0;

	if (manager->OnLButtonUp(ptPos))
	{
		PlaySound((LPCSTR)IDR_WAVE_PUTDOWN, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

		if (manager->GetIsWon())
			manager->Win();

		RefreshMenu();

	}
	return 0;
}

LRESULT MainWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);

	if (manager->bOnThread)
		return 0;

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