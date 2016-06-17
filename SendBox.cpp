#include "stdafx.h"
#include "commctrl.h"
#include "resource.h"
#include "BoxHelper.h"
#include "SendBox.h"
#include "UART.h"

static HANDLE hThread;
static HANDLE hCurrUART;
static FILE* fpSend;
static char* pzSendFile;
static DWORD dwFileSize;
static DWORD dwFileSend;

//WPARAM 状态
//LPARAM 数据长度
#define WM_SENDFILE		WM_USER+101

static DWORD WINAPI SendFileThread(void* arg)
{
	size_t ret;
	HWND hWnd;
	DWORD dwSend;
	char biSendBuff[256];
	hWnd = (HWND)arg;
	
	fseek(fpSend,0,SEEK_SET);
	while(1)
	{
		ret = fread(biSendBuff,1,256,fpSend);
		dwSend=WriteUART(hCurrUART,biSendBuff,(DWORD)ret);
		SendMessage(hWnd,WM_SENDFILE,1,dwSend);
		if(dwSend != ret)
		{
			SendMessage(hWnd,WM_SENDFILE,0,1);
			break;
		}
		else if(ret < 256)
		{
			SendMessage(hWnd,WM_SENDFILE,0,0);
			break;
		}
		Sleep(2);
	}
	ExitThread(1);
}

static INT_PTR WINAPI SendBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char szText[32];
	DWORD dwPID;
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hWnd,IDC_PROG,PBM_SETRANGE,0,MAKELPARAM(0,100));
		SendDlgItemMessage(hWnd,IDC_PROG,PBM_SETPOS,0,0);
		SetDlgItemText(hWnd,IDC_TEXT_FILE,pzSendFile);
		
		fpSend = fopen(pzSendFile,"rb");
		if(fpSend==NULL)
		{
			SetDlgItemText(hWnd,IDC_TEXT_STATE, "打开文件失败!");
			break;
		}
		fseek(fpSend,0,SEEK_END);
		dwFileSend=0;
		dwFileSize=ftell(fpSend);
		
		SetDlgItemText(hWnd,IDC_TEXT_STATE, "正在发送...");
		hThread = CreateThread(NULL,0,SendFileThread, hWnd, 0, &dwPID);
		break;
	case WM_SENDFILE:
		if(wParam==0)
		{
			switch(lParam)
			{
			case 0:
				SetDlgItemText(hWnd,IDC_TEXT_STATE, "发送完成!");
				break;
			case 1:
				SetDlgItemText(hWnd,IDC_TEXT_STATE, "发送失败!");
				break;
			}
		}
		else
		{
			dwFileSend += (DWORD)lParam;
			sprintf(szText,"%d/%d 字节",dwFileSend,dwFileSize);
			SetDlgItemText(hWnd,IDC_TEXT_RATE,szText);
			SendDlgItemMessage(hWnd,IDC_PROG,PBM_SETPOS,(dwFileSend*100)/dwFileSize,0);
		}
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
		case IDCANCEL:
			TerminateThread(hThread,0);
			CloseHandle(hThread);
			EndDialog(hWnd,0);
			break;
		}
		break;
	}
	return 0;
}

void PopupSendBox(HINSTANCE hInst, HWND hParent, HANDLE hUART, char* pzName)
{
	hCurrUART = hUART;
	pzSendFile= pzName;

	DialogBox(hInst,MAKEINTRESOURCE(IDD_SENDFILE), hParent, SendBoxProc);
}
