#pragma once
#define STRICT
#define VC_EXTRALEAN

#include <atlbase.h>        // ������ATL��
extern CComModule _Module;  // ȫ��_Module
#include <atlwin.h>         // ATL������

#include <vector>
#include <queue>
#include <Windowsx.h>

#include "resource.h"

#pragma comment(lib,"comctl32.lib")


#include "Manager.h"

#define TIPBOX_WIDTH 200
#define TIPBOX_HEIGHT 100
#define BORDER 10

extern COLORREF crTipBox;
extern int cardWidth;
extern int cardHeight;
extern int cardGapH;