#include "DialogAuto.h"

#include <thread>
#include <string>

using namespace std;

LRESULT DialogAuto::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	btnStart.LinkControl(m_hWnd, IDC_BUTTON_START);
	checkboxAnimation.LinkControl(m_hWnd, IDC_CHECK_ANIMATION);
	CenterWindow();
	return TRUE;    // let the system set the focus
}

LRESULT DialogAuto::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (manager->bOnThread)
	{
		manager->bStopThread = true;
		PostMessage(WM_CLOSE);
	}
	else
		EndDialog(0);
	return 0;
}

LRESULT DialogAuto::OnBtnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	auto fun = [&]()
	{
		::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START),"ֹͣ");
		manager->Command("auto "+to_string(checkboxAnimation.GetChecked()));
		::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START), "��ʼ");
	};

	if (manager->bOnThread == false)
	{
		thread t(fun);
		t.detach();
	}
	else
	{
		manager->bStopThread = true;
		::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START), "��ʼ");
	}
	return 0;
}