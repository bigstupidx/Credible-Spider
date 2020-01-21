#pragma once
#include "stdafx.h"
#include "TImage.h"
#include "Manager.h"
#include "Draw.h"

#include <memory>

//class RectShadow;
class MainWindow :public CWindowImpl<MainWindow, CWindow, CFrameWinTraits>
{
private:
	std::shared_ptr<Manager> manager;

	const int border = 10;

	class TipBox
	{
	private:
		const int tipBoxWidth = 200;
		const int tipBoxHeight = 100;
		const COLORREF crTipBox = 0x00007f00;
		std::string textTipBox;
		HBRUSH hBrushTipBox;
		RECT rcTipBox;
	public:
		TipBox() :hBrushTipBox(CreateSolidBrush(crTipBox)) {}

		~TipBox()
		{
			DeleteObject(hBrushTipBox);
		}

		void OnSize(const RECT& rcClient, int border)
		{
			//ˢ����ʾ��λ��
			rcTipBox.left = (rcClient.right - tipBoxWidth) / 2;
			rcTipBox.right = rcTipBox.left + tipBoxWidth;
			rcTipBox.bottom = rcClient.bottom - border;
			rcTipBox.top = rcTipBox.bottom - tipBoxHeight;
		}

		void Draw(HDC hdc,std::shared_ptr<Manager> manager)
		{
			if (manager->HasPoker())
			{
				textTipBox = "������" + std::to_string(manager->GetPokerScore()) + "\r\n";
				textTipBox += "������" + std::to_string(manager->GetPokerOperation());
			}

			SelectObject(hdc, GetStockPen(BLACK_PEN));
			SelectObject(hdc, hBrushTipBox);
			Rectangle(hdc, rcTipBox.left, rcTipBox.top, rcTipBox.right, rcTipBox.bottom);

			Draw::DrawTextCenter(hdc, textTipBox.c_str(), rcTipBox, 12, 400, RGB(255, 255, 255), TEXT("����"), DT_LEFT);
		}

		const RECT& GetRect()
		{
			return rcTipBox;
		}
	};

	TipBox tipBox;

	//RectShadow* rectShadow;

	RECT rcClient;
	HDC hdcMem;
	HDC hdcBackground;
	TImage *imgBackground;
public:
	bool doubleBuffer;
	DECLARE_WND_CLASS(_T("Credible Spider Window"))

	BEGIN_MSG_MAP(MainWindow)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_CLOSE,OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		COMMAND_ID_HANDLER(ID_NEW_GAME, OnNewGame)
		COMMAND_ID_HANDLER(ID_RENEW_GAME, OnReNewGame)
		COMMAND_ID_HANDLER(ID_RELEASE, OnRelease)
		COMMAND_ID_HANDLER(ID_RELEASE2, OnRelease)
		COMMAND_ID_HANDLER(ID_SHOWMOVE, OnShowMove)
		COMMAND_ID_HANDLER(ID_SCORE, OnHighScore)
		COMMAND_ID_HANDLER(ID_REDO, OnRedo)
		COMMAND_ID_HANDLER(ID_AUTO, OnAuto)
		COMMAND_ID_HANDLER(ID_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_QUIT, OnQuit)
		COMMAND_ID_HANDLER(ID_ENABLE_ANIMATION, OnSetOption)
		COMMAND_ID_HANDLER(ID_ENABLE_SOUND, OnSetOption)
	END_MSG_MAP()

	MainWindow() :doubleBuffer(true), imgBackground(nullptr) {}

	LRESULT MainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void MainWindow::Draw(HDC hdc, const RECT &rect);

	//�ڶ��̶߳���ʱ���϶����ڻ��������������Ĭ������»ᷢ����ɫ��˸��
	//����WM_ERASEBKGND������false������Ϊ��������������˸
	LRESULT MainWindow::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT MainWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT MainWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void EnableAllInput(bool enable);
	void MainWindow::RefreshMenu();

	LRESULT MainWindow::OnNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnReNewGame(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnRelease(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnShowMove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnHighScore(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnRedo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnAuto(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnQuit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT MainWindow::OnSetOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
