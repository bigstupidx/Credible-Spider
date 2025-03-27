#include "DialogAuto.h"

#include <thread>
#include <string>

using namespace std;

LRESULT DialogAuto::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    btnStart.LinkControl(m_hWnd, IDC_BUTTON_START);
    checkboxAnimation.LinkControl(m_hWnd, IDC_CHECK_ANIMATION);

    RECT parentRect;
    GetParent().GetClientRect(&parentRect);

    RECT rect;
    GetClientRect(&rect);

    POINT dest = { 10, parentRect.bottom - rect.bottom - 10 };
    ::ClientToScreen(GetParent().m_hWnd, &dest);

    SetWindowPos(HWND_TOPMOST, dest.x, dest.y, rect.right, rect.bottom, SWP_NOSIZE);

    ret = make_shared<RetData>();
    *ret = { false, 0 };

    //CenterWindow();
    return TRUE;    // let the system set the focus
}

LRESULT DialogAuto::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (manager->bOnThread) {
        manager->bStopThread = true;
        PostMessage(WM_CLOSE);
    } else {
        manager->bStopThread = false;
        EndDialog(0);
    }
    return 0;
}

LRESULT DialogAuto::OnBtnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    auto fun = [&]() {
        ::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START), "ֹͣ");
        checkboxAnimation.SetEnable(false);
        if (manager->Command("auto " + to_string(checkboxAnimation.GetChecked()))) {
            //�ɹ����Զ��˳�
            PostMessage(WM_CLOSE);
        }

        stringstream ss;
        ss << "��������" << endl;
        ss << "�����" << (manager->autoSolveResult.success ? "�ɹ�" : "ʧ��") << endl;
        ss << "���Դ�����" << manager->autoSolveResult.calc;

        ret->solved = manager->autoSolveResult.success;
        ret->calc += manager->autoSolveResult.calc;

        MessageBox(ss.str().c_str(), "�����", MB_OK | MB_ICONINFORMATION);


        ::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START), "��ʼ");
        checkboxAnimation.SetEnable(true);
    };

    if (manager->bOnThread == false) {
        thread t(fun);
        t.detach();
    } else {
        manager->bStopThread = true;
        ::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON_START), "��ʼ");
    }
    return 0;
}