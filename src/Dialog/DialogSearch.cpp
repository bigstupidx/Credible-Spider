#include "DialogSearch.h"

#include "Manager.h"
#include "TStatic.h"
#include <string>
#include <thread>

using namespace std;

LRESULT DialogSearch::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    pSeed = make_shared<uint32_t>();
    pCalc = make_shared<int>();

    btnCancel.LinkControl(m_hWnd, IDCANCEL);

    SetWindowText("�����������н�...");

    //ȡ��CPU�߼�������
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    coreNum = sysInfo.dwNumberOfProcessors;
    coreNum = max(coreNum / 2, 1);

    int fontHeight = 18;
    int border = 10;
    int dHeight = (coreNum + 1) * fontHeight;
    RECT rectClient;
    GetClientRect(&rectClient);

    RECT rect;
    GetWindowRect(&rect);
    int captionHeight = (rect.bottom - rect.top) - rectClient.bottom;
    rect.bottom += dHeight;
    MoveWindow(&rect);

    RECT rectBtn;
    ::GetWindowRect(btnCancel.GetHWND(), &rectBtn);
    int x = rectBtn.left - rect.left;
    int y = rectBtn.top - rect.top - captionHeight;
    int w = rectBtn.right - rectBtn.left;
    int h = rectBtn.bottom - rectBtn.top;
    y += dHeight + border;
    ::MoveWindow(btnCancel.GetHWND(), x, y, w, h, 1);

    int width = rect.right - rect.left;
    staticMemo.Create(m_hWnd, { border, border, width, fontHeight });
    staticMemo.SetText("�߳�������" + to_string(coreNum));
    staticMemo.SetDefaultGuiFont();

    for (int i = 0; i < coreNum; ++i) {
        vecStatic.emplace_back();
        vecStatic.back().Create(m_hWnd, { border, border + fontHeight * (i + 1), width, fontHeight });
        vecStatic.back().SetText("�߳�" + to_string(i));
        vecStatic.back().SetDefaultGuiFont();
    }

    CenterWindow();

    PostMessage(WM_COMMAND, MAKELONG(IDOK, 0));

    return TRUE;    // let the system set the focus
}

LRESULT DialogSearch::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (IsManagerOnThread() || searchThreadIsRunning) {
        StopManagerThread();
        stopThread = true;
        PostMessage(WM_CLOSE);
        return 0;

    }

    pSeed = nullptr;
    pCalc = nullptr;
    EndDialog(0);
    return 0;
}

bool DialogSearch::IsManagerOnThread() {
    for (auto& manager : vecManager)
        if (manager && manager->bOnThread) {
            return true;
        }
    return false;
}

void DialogSearch::StopManagerThread() {
    for (auto& manager : vecManager)
        if (manager->bOnThread) {
            manager->bStopThread = true;
        }
}

LRESULT DialogSearch::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    int id = LOWORD(wParam);

    switch (id) {
    case IDOK: {
        //���߳�
        auto fun = [&]() {
            searchThreadIsRunning = true;
            stopThread = false;

            //���������߳�
            auto search = [&](std::shared_ptr<Manager>& manager) {
                while (1) {
                    //����������ƾ�
                    manager->Command("newrandom " + to_string(suit));
                    if (manager->Command("auto")) {
                        //�������
                        *pSeed = manager->GetPokerSeed();
                        break;
                    }

                    //Ҫ��ֹͣ�����˳��߳�
                    if (manager->bStopThread)
                        break;
                }
            };

            //����manager
            vecManager = vector<shared_ptr<Manager>>(coreNum, nullptr);

            //ÿ��manager���ο���
            for (int i = 0; i < coreNum; ++i) {
                auto& manager = vecManager[i];
                manager = make_shared<Manager>();
                manager->SetTextOutputHWND(vecStatic[i].GetHWND());
                manager->bOnThread = true;

                thread t(search, manager);
                t.detach();
            }

            //�б�
            shared_ptr<Manager> resultManager;
            while (IsManagerOnThread() || stopThread == false) {
                this_thread::sleep_for(10ms);
                bool found = false;

                //Ѱ���Ƿ����߳������
                for (auto& manager : vecManager)
                    if (manager->autoSolveResult.success == true) {
                        //�õ����
                        resultManager = manager;
                        found = true;
                        break;
                    }

                if (found)
                    break;
            }

            //ֹͣ���������߳�
            StopManagerThread();

            if (resultManager) {
                stringstream ss;
                ss << "�ҵ�һ����н⣺" << endl << endl;
                ss << "��ɫ=" << resultManager->autoSolveResult.suit << endl;
                ss << "����=" << resultManager->autoSolveResult.seed << endl;
                ss << "���Դ���=" << resultManager->autoSolveResult.calc << endl;
                ss << "�Ѷ�=" << -10000.0 / resultManager->autoSolveResult.calc + 100.0 << endl;

                *pCalc = resultManager->autoSolveResult.calc;

                MessageBox(ss.str().c_str(), "�����", MB_OK | MB_ICONINFORMATION);
                searchThreadIsRunning = false;

                EndDialog(0);
            }
            searchThreadIsRunning = false;
        };

        thread t(fun);
        t.detach();
        break;
    }
    case IDCANCEL:

        SendMessage(WM_CLOSE);
        break;
    }

    return 0;
}