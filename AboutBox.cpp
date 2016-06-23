#include "stdafx.h"
#include "resource.h"
#include "BoxHelper.h"

static void OnInitDialog(HWND hWnd)
{
	SetWindowText(hWnd,"����");
	SetDlgItemText(hWnd,IDC_TEXT_CAPTION,"���ڵ��Թ���");
	SetDlgItemText(hWnd,IDC_TEXT_VERSION,"�汾��2.0.4");
	SetDlgItemText(hWnd,IDC_TEXT_DATE,"���ڣ�2016.06.23");
	SetDlgItemText(hWnd,IDC_TEXT_EMAIL,"���ߣ�kerndev@foxmail.com");
	SetDlgItemText(hWnd,IDC_EDIT_LIC,"���������������,�����⸴�ƴ���,���ʹ�á��벻Ҫ�Ա�������ж����޸Ļ���������ҵĲ��Ŀ��.\r\n");
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
