#include "MainWindow.h"

#include "Dialog/DialogChooseLevel.h"
#include "ADrawCard.h"
#include "ASetCard.h"
#include "ACommand.h"
#include "AModifyVecData.h"
#pragma comment(lib,"winmm.lib")

using namespace std;

HWND MainWindow::s_hWnd = NULL;
std::string MainWindow::textTipBox;
std::vector<POINT> MainWindow::vecCorner;


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
	HMENU hmenu = LoadMenu(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDR_MENU_MAIN));
	SetMenu(hmenu);

	HICON hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON));
	SetIcon(hIcon, FALSE);

	imgBackground = new TImage(GetModuleHandle(NULL), IDB_BACKGROUND);
	imgCardEmpty = new TImage(GetModuleHandle(NULL), IDB_CARDEMPTY);
	imgCardBack = new TImage(GetModuleHandle(NULL), IDB_CARDBACK);

	int id_card_first = IDB_CARD1;
	for (int i = 0; i < 52; ++i)
	{
		imgCard[i] = new TImage(GetModuleHandle(NULL), id_card_first + i);
	}

	hBrushTipBox = CreateSolidBrush(crTipBox);

	cardEmpty = true;

	bOnAnimation = false;

	bOnDrag = false;

	//manager.pCmdFunc = &RefreshByManager;

	s_hWnd = m_hWnd;

	PostMessage(WM_COMMAND, MAKELONG(ID_NEW_GAME, 0), 0);
	return 0;
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	auto answer = MessageBox(TEXT("关闭该游戏之前是否需要保存？"), TEXT("Credible Spider"), MB_YESNOCANCEL | MB_ICONQUESTION);
	switch (answer)
	{
	case IDYES:
	case IDNO:
		DestroyWindow();
	case IDCANCEL:
		break;
	}
	return 0;
}

LRESULT MainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	delete imgBackground;
	delete imgCardEmpty;
	delete imgCardBack;
	for (auto &imgC : imgCard)
		delete imgC;

	DeleteObject(hBrushTipBox);
	PostQuitMessage(0);
	return 0;
}

void PlaySoundDeal(void *p)
{
		PlaySound((LPCSTR)IDR_WAVE_DEAL, GetInstanceModule(NULL), SND_RESOURCE | SND_SYNC);
}

LRESULT MainWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD id = LOWORD(wParam);
	if (qAnimation.empty())
	{
		//mciSendString("CLOSE MUSIC", NULL, 0, NULL);
		KillTimer(id);
		bOnAnimation = false;
		Invalidate(false);
	}
	else
	{
		//mciSendString("PLAY MUSIC FROM 0", NULL, 0, NULL);

		Invalidate(false);
	}
	return 0;
}

int GetIndexFromSuitPoint(int suit, int point)
{
	return (suit - 1) * 13 + point - 1;
}

void MainWindow::Draw(HDC hdc, const RECT &rect)
{
	//Background
	imgBackground->Fill(hdc, rect);

	//TipBox

	SelectObject(hdc, hBrushTipBox);
	Rectangle(hdc, rectTipBox.left, rectTipBox.top, rectTipBox.right, rectTipBox.bottom);

	DrawTextCenter(hdc, textTipBox.c_str(), rectTipBox, 12, 400, RGB(255, 255, 255), TEXT("宋体"), DT_LEFT);

	//Card Empty
	if (cardEmpty)
	{
		for (auto &pt : vecDesk)
			imgCardEmpty->Draw(hdc, pt.x, pt.y);
	}


	for (auto &vec : vecCard)
	{
		for (auto &card : vec)
		{
			if (card.show)
				card.img->Draw(hdc, card.pt.x, card.pt.y);
		}
	}

	//Draw corner
	for (auto &pt : vecCorner)
		imgCardBack->Draw(hdc, pt.x, pt.y);

	if (!qAnimation.empty())
	{
		qAnimation.front()->Do(hdc);
		delete qAnimation.front();
		qAnimation.pop();
		//SendMessage(WM_SIZE);
	}

	//if (bTimerDeal)
	//	imgCard[animateCard.first]->Draw(hdc, animateCard.second.x, animateCard.second.y);
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

void MainWindow::RefreshCard()
{
	//Card
	vecCard.clear();
	int col = 0;
	for (auto &deskCards : manager->GetPoker()->desk)
	{
		std::vector <CardDrawer> temp;
		int x = vecDesk[col].x;
		int y = vecDesk[col].y;
		for (auto &card : deskCards)
		{
			if (card.show)
			{
				int index = GetIndexFromSuitPoint(card.suit, card.point);
				temp.push_back({ imgCard[index],true, { x, y } });

					y += cardGapH*2;
			}
			else
			{
				temp.push_back({ imgCardBack, true, { x, y } });

					y += cardGapH;
			}
		}
		vecCard.push_back(temp);
		col++;
	}
}

LRESULT MainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rect;
	GetClientRect(&rect);

	//刷新提示框位置
	rectTipBox.left = (rect.right - TIPBOX_WIDTH) / 2;
	rectTipBox.right = rectTipBox.left + TIPBOX_WIDTH;
	rectTipBox.bottom = rect.bottom - BORDER;
	rectTipBox.top = rectTipBox.bottom - TIPBOX_HEIGHT;

	//刷新牌位置
	vecDesk.resize(10);
	int cardGap = ((rect.right - BORDER * 2) - cardWidth * 10) / 9;
	for (int i = 0; i < 10; ++i)
	{
		vecDesk[i].x = BORDER + i*(cardWidth + cardGap);
		vecDesk[i].y = BORDER;
	}

	//Card
	//只更新位置
	int col = 0;
	for (auto &cards : vecCard)
	{
		int x = vecDesk[col].x;
		int y = vecDesk[col].y;
		for (auto it = cards.begin(); it != cards.end();++it)
		{
			it->pt.x = x;
			it->pt.y = y;
			if (it->img != imgCardBack)
				y += cardGapH * 2;
			else
				y += cardGapH;
		}
		col++;
	}

	//刷新堆牌
	if (manager && manager->GetPoker())
	{
		vecCorner.resize(manager->GetPoker()->corner.size());
		for (int i = 0; i < vecCorner.size(); ++i)
		{
			vecCorner[i].x = rect.right - BORDER - cardWidth - i*BORDER;
			vecCorner[i].y = rect.bottom - BORDER - cardHeight;
		}
	}
	return 0;
}

void MainWindow::RedoDealAnimation()
{
	RECT rect;
	GetClientRect(&rect);

	int destX;
	int destY;
		destX = rect.right - BORDER - cardWidth-BORDER*(vecCorner.size());
		destY = rect.bottom - BORDER - cardHeight;

	Animation *tempAni;

	int deskNum = 10;
	for (int i = 0; i < deskNum; ++i)
	{

		//得到此牌动画的起止位置
		int origX = vecCard[i].back().pt.x;
		int origY = vecCard[i].back().pt.y + cardGapH;
		CardDrawer *card = &vecCard[i].back();

		//插值10帧进行动画
		int frames = 5;
		for (int f = 0; f < frames; ++f)
		{
			//插值
			int x = (destX - origX)*f / frames + origX;
			int y = (destY - origY)*f / frames + origY;

			//加入动画序列
			tempAni = new ADrawCard(card->img, { x, y });
			qAnimation.push(tempAni);
		}
		//显示此牌
		tempAni = new ASetCard(card, false);
		qAnimation.push(tempAni);

		//增加堆牌
		if (i==0)
			qAnimation.push(new AModifyVecData(&vecCorner, { destX, destY }));
	}

	tempAni = new ACommand("redo", manager.get());
	qAnimation.push(tempAni);

	//定时器开始，每次定时刷新pop一下动画序列
	bOnAnimation = true;
	SetTimer(TIMER_DEAL, TIMER_DEAL_ELPS);
	//mciSendString("OPEN deal.wav ALIAS MUSIC", NULL, 0, 0);
	PlaySound((LPCSTR)IDR_WAVE_DEAL, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
}

void MainWindow::AddDealAnimation()
{
	//全程不对poker进行改动

	//最上面10张牌设为隐藏
	vector<CardDrawer *> temp;
	for (auto &card : vecCard)
	{
		if (!card.empty())
		{
			temp.push_back(&card.back());
			card.back().show = false;
		}
	}

	int origX = vecCorner.back().x;
	int origY = vecCorner.back().y;

	Animation *tempAni;
	//tempAni = new AModifyVecData(&vecCorner, { origX, origY });
	//qAnimation.push(tempAni);

	for (int i = 0; i < temp.size(); ++i)
	{
		//发最后一张时，减去堆牌
		if (i == temp.size() - 1)
		{
			tempAni = new AModifyVecData(&vecCorner);
			qAnimation.push(tempAni);
		}

		//得到此牌动画的起止位置
		int destX = vecCard[i].back().pt.x;
		int destY = vecCard[i].back().pt.y + cardGapH;
		CardDrawer *card = temp[i];

		//插值10帧进行动画
		int frames = 5;
		for (int f = 0; f < frames; ++f)
		{
			//插值
			int x = (destX - origX)*f / frames + origX;
			int y = (destY - origY)*f / frames + origY;

			//加入动画序列
			tempAni = new ADrawCard(card->img, { x, y });
			qAnimation.push(tempAni);
		}
		//显示此牌
		tempAni = new ASetCard(card, true);
		qAnimation.push(tempAni);
	}

	//定时器开始，每次定时刷新pop一下动画序列
	bOnAnimation = true;
	SetTimer(TIMER_DEAL, TIMER_DEAL_ELPS);
	//mciSendString("OPEN deal.wav ALIAS MUSIC", NULL, 0, 0);
	PlaySound((LPCSTR)IDR_WAVE_DEAL, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
}


LRESULT MainWindow::OnReNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IDYES == MessageBox("是否重新开始本局游戏？", "询问", MB_YESNO))
	{
		//洗牌
		manager->Command("d " + std::to_string(manager->GetPoker()->suitNum) + " " + std::to_string(manager->GetPoker()->seed));

		//隐藏空白框
		cardEmpty = false;

		//初始化牌位置
		RefreshCard();

		//初始化各牌堆位置
		SendMessage(WM_SIZE);

		//牌堆增加1个显示项
		int origX = vecCorner.back().x - BORDER;
		int origY = vecCorner.back().y;
		vecCorner.push_back({ origX, origY });

		AddDealAnimation();

		Invalidate(false);

		EnableMenuItem(GetMenu(), ID_DEAL, MF_ENABLED);
	}
	return 0;
}

LRESULT MainWindow::OnNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DialogChooseLevel dialogChooseLevel;
	int suit = dialogChooseLevel.DoModal();
	if (suit != 0)
	{
		//洗牌
		manager = make_shared<Manager>(suit);
		//manager->Command("dr " + std::to_string(suit));

		//隐藏空白框
		cardEmpty = false;

		//初始化牌位置
		RefreshCard();

		//初始化各牌堆位置
		SendMessage(WM_SIZE);

		//牌堆增加1个显示项
		int origX = vecCorner.back().x - BORDER;
		int origY = vecCorner.back().y;
		vecCorner.push_back({ origX, origY });

		AddDealAnimation();

		Invalidate(false);
		
		EnableMenuItem(GetMenu(),ID_DEAL, MF_ENABLED);

		EnableMenuItem(GetMenu(), ID_RENEW_GAME, MF_ENABLED);
	}

	return 0;
}

void MainWindow::RefreshByManager()
{
	//刷新 撤销 命令
	if (manager->CanRedo())
		EnableMenuItem(::GetMenu(s_hWnd), ID_REDO, MF_ENABLED);
	else
		EnableMenuItem(::GetMenu(s_hWnd), ID_REDO, MF_DISABLED);

	//刷新 发牌 命令
	if (manager->GetPoker()->corner.empty())
	{
		EnableMenuItem(::GetMenu(s_hWnd), ID_DEAL, MF_DISABLED);
	}
	else
		EnableMenuItem(::GetMenu(s_hWnd), ID_DEAL, MF_ENABLED);


	textTipBox = "分数：" + std::to_string(manager->GetPoker()->score) + "\r\n";
	textTipBox += "操作：" + std::to_string(manager->GetPoker()->operation);

	//RECT rect;
	//::GetClientRect(s_hWnd,&rect);

}

LRESULT MainWindow::OnDeal(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	manager->Command("r");
	RefreshCard();

	AddDealAnimation();
	return 0;
}

LRESULT MainWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);
	if (!bOnAnimation)
	{

		//发牌
		if (!vecCorner.empty())
		{
			//得到牌堆顶端牌RECT
			RECT rectCorner;
			rectCorner.left = vecCorner.back().x;
			rectCorner.top = vecCorner.back().y;
			rectCorner.right = rectCorner.left + cardWidth;
			rectCorner.bottom = rectCorner.top + cardHeight;

			//发牌
			BOOL b;
			if (PtInRect(&rectCorner, ptPos))
				OnDeal(NULL, NULL, NULL, b);
		}

		//提示
		if (PtInRect(&rectTipBox, ptPos))
		{
			PlaySound(LPCSTR(IDR_WAVE_TIP), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		}

		//移牌
		int col, row;
		if (GetPtOnCard(ptPos, col, row))
		{
			if (manager->Command("pick " + to_string(col) + " " + to_string(row)))
			{
				bOnDrag = true;
				dragCard.clear();

				for (int j = row; j < vecCard[col].size(); ++j)
				{
					dragCard.push_back({ &vecCard[col][j], vecCard[col][j].pt });
				}

				ptDragStart = ptPos;
				ptChange = { 0, 0 };
				PlaySound((LPCSTR)IDR_WAVE_PICKUP, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
			}
		}

	}
	return 0;
}

LRESULT MainWindow::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bOnDrag = false;
	return 0;
}

bool MainWindow::PtInCard(POINT ptMouse, POINT ptCard)
{
	return (ptMouse.x >= ptCard.x && ptMouse.x <= ptCard.x + cardWidth &&
		ptMouse.y >= ptCard.y && ptMouse.y <= ptCard.y + cardHeight);
}

bool MainWindow::GetPtOnCard(POINT ptMouse, int &col, int &row)
{
	col = -1, row = -1;
	for (auto it1 = vecCard.begin(); it1 != vecCard.end(); ++it1)
	{
		for (auto it = it1->rbegin(); it != it1->rend(); ++it)
		{
			if (it->show && it->img != imgCardBack && PtInCard(ptMouse, it->pt))
			{
				col = it1 - vecCard.begin();
				row = it1->size() - 1 - (it - it1->rbegin());
				break;
			}
		}
	}
	return (col != -1 && row != -1);
}

LRESULT MainWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT ptPos;

	ptPos.x = LOWORD(lParam);
	ptPos.y = HIWORD(lParam);
	string s="";
	//int col = -1, row = -1;
	//GetPtOnCard(ptPos, col, row);
	//s = to_string(col) + "," + to_string(row);

	if (bOnDrag)
	{
		ptChange.x = ptPos.x - ptDragStart.x;
		ptChange.y = ptPos.y - ptDragStart.y;

		s = to_string(ptChange.x) + "," + to_string(ptChange.y);
		for (auto &pCard : dragCard)
		{
			pCard.first->pt.x = pCard.second.x + ptChange.x;
			pCard.first->pt.y = pCard.second.y + ptChange.y;
		}
		Invalidate(false);
	}

	SetWindowText(s.c_str());
	return 0;
}


LRESULT MainWindow::OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//if (!bOnAnimation)
	//if (manager->GetLastAct() != NULL && manager->GetLastAct()->GetCommand() == "r")
	//{
	//	RedoDealAnimation();
	//}
	//manager.Command("redo");
	return 0;
}