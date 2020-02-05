#pragma once
#include <Windows.h>

#include <vector>
#include <string>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

#include "TControl.h"

class TListView:public TControl
{
private:
public:
	TListView();
	TListView(const TListView& listView);
	TListView& operator=(const TListView& listView);

//����ListView
	void TListView::CreateListViewEx(HWND hParent, UINT id, HINSTANCE hInst);

	void SetFullRowSelect(bool enable);

//����б�ǩ
	void TListView::AddColumn(TCHAR text[], int width, int styleLVCFMT = LVCFMT_LEFT);

//�����б�ǩ
	void TListView::InsertColumn(int index, TCHAR text[], int width, int styleLVCFMT = LVCFMT_LEFT);

//������Ŀ
	void AddItem(std::vector<std::string> items);

	void UpdateItem(int index, std::vector<std::string> items);

	void SetItem(int index, int col,std::tstring text);

	std::tstring GetItem(int index, int col);

	std::vector<int> GetCurSel();

	int TListView::GetColumnCount();

	int TListView::GetItemCount();//�õ���Ŀ��

	void TListView::DeleteAllColumns();//���ȫ����
	void TListView::DeleteAllItems();//���ȫ����

	RECT TListView::GetGridRect(int index, int subitem);//�����кź��кŵõ���������

	RECT TListView::GetGridRectInMargin(int index, int subitem);//�õ����ӱ߿��ڵ�����
};

