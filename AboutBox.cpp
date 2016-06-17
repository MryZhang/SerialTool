#include "stdafx.h"
#include "resource.h"
#include "BoxHelper.h"

static void OnInitDialog(HWND hWnd)
{
	SetWindowText(hWnd,"关于");
	SetDlgItemText(hWnd,IDC_TEXT_CAPTION,"串口调试工具");
	SetDlgItemText(hWnd,IDC_TEXT_VERSION,"版本：2.0.3");
	SetDlgItemText(hWnd,IDC_TEXT_DATE,"日期：2016.05.28");
	SetDlgItemText(hWnd,IDC_TEXT_EMAIL,"作者：kerndev@foxmail.com");
	SetDlgItemText(hWnd,IDC_EDIT_LIC,"本软件是免费软件，你可以任意复制、传播和使用本软件。\r\n");
}

INT_PTR WINAPI AboutProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		OnInitDialog(hWnd);
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;
	}
	return 0;
}

void PopupAboutBox(HINSTANCE hInst, HWND hParent)
{
	static BOOL bLock;
	if(bLock==0)
	{
		bLock = 1;
		DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUTBOX),hParent,AboutProc);
		bLock = 0;
	}
}
