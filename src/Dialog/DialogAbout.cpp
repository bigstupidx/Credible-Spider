#include "DialogAbout.h"

#include <string>

using namespace std;
LRESULT DialogAbout::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	edit.LinkControl(m_hWnd, IDC_EDIT);

	string s =  
"���Զ�����ʹ�û��ݷ�+������+��ֵ����+Hash�ظ������ж�ʵ�֡�\r\n\r\n"
"���Զ������ܽ��90%���ϵ�1��ɫ�ƾ֡�20%���ҵ�2��ɫ�ƾ֣�4��ɫ���ܽ����\r\n\r\n"
"         �׸���������ӡ�\r\n\r\n"
"         ���ߣ�Tom Willow\r\n"
"         tomwillow@qq.com\r\n\r\n"
"https://github.com/tomwillow/Credible-Spider";
	edit.SetText(s);

	return 0;
}