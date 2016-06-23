#include "stdafx.h"
#include "commctrl.h"
#include "resource.h"
#include "BoxHelper.h"
#include "SerialTool.h"
#include "UART.h"
#include "EnumUART.h"
#include "convert.h"
#include "AboutBox.h"
#include "FileBox.h"
#include "SendBox.h"
#include "CbtHook.h"

#define MAX_TEXT_LEN	(256*1024)
#define MAX_DATA_LEN	(64*1024)

//接收数据消息
//WPARAM:数据长度
//LPARAM:数据指针
#define MSG_RECVDATA	WM_USER+100

static HINSTANCE hInst;
static BOOL bInit;
static RECT MinRect;
static RECT CurRect;
static HANDLE hUART;
static HANDLE hThread;
static BOOL bAutoClear;
static BOOL bAutoSend;
static BOOL bHEXSend;
static BOOL bHEXDisp;
static BOOL bRecvFile;
static FILE* fpRecvFile;
static DWORD dwSendCount;
static DWORD dwRecvCount;
static char  szSendBuff[MAX_TEXT_LEN+1];
static char  szRecvBuff[MAX_TEXT_LEN+1];
static BYTE  biSendBuff[MAX_DATA_LEN+1];
static BYTE  biRecvBuff[MAX_DATA_LEN+1];
static DWORD dwRecvBuffCount;


static void FillBaudRate(HWND hWnd)
{
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"1200");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"2400");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"4800");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"9600");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"14400");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"19200");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"38400");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"57600");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"115200");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"230400");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"460800");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"921600");
	SendMessage(hWnd,CB_SETCURSEL,8,0);
}

static void FillSerialPort(HWND hWnd)
{
	int i;
	int count;
	SendMessage(hWnd,CB_RESETCONTENT,0,0);
	count=EnumUART();
	for(i=0;i<count;i++)
	{
		SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)GetPortName(i));
	}
	SendMessage(hWnd,CB_SETCURSEL,0,0);
}

static void FillDataBits(HWND hWnd)
{
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"5");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"6");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"7");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"8");
	SendMessage(hWnd,CB_SETCURSEL,3,0);
}

static void FillStopBits(HWND hWnd)
{
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"1");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"1.5");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"2");
	SendMessage(hWnd,CB_SETCURSEL,0,0);
}

static void FillParityBit(HWND hWnd)
{
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"None");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"Odd");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"Even");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"Mark");
	SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)"Space");
	SendMessage(hWnd,CB_SETCURSEL,0,0);
}


static void AppendRecvData(HWND hWnd, char* pzText)
{
	HWND hItem;
	LONG lStart;
	LONG lEnd;
	LRESULT lTotalLen;

	hItem = GetDlgItem(hWnd,IDC_EDIT_RECV);
	lTotalLen=SendMessage(hItem,WM_GETTEXTLENGTH,0,0);
	SendMessage(hItem,EM_GETSEL,(WPARAM)&lStart,(LPARAM)&lEnd);
	if(lStart != lEnd)
	{
		SendMessage(hItem,WM_SETREDRAW,0,0);
	}
	SendMessage(hItem,EM_SETSEL,lTotalLen,lTotalLen);
	SendMessage(hItem,EM_REPLACESEL,0,(LPARAM)pzText);
	if(lStart != lEnd)
	{
		SendMessage(hItem,EM_SETSEL,lStart,lEnd);
		SendMessage(hItem,WM_SETREDRAW,1,1);
	}
}

static void ClearRecvData(HWND hWnd)
{
	dwRecvBuffCount = 0;
	biRecvBuff[0] = 0;
	SetDlgItemText(hWnd,IDC_EDIT_RECV,"");
}

static void UpdateDataCounter(HWND hWnd)
{
	char szText[32];
	sprintf(szText,"发送:%d",dwSendCount);
	SetDlgItemText(hWnd,IDC_TEXT_SENDCOUNT,szText);
	sprintf(szText,"接收:%d",dwRecvCount);
	SetDlgItemText(hWnd,IDC_TEXT_RECVCOUNT,szText);
}

static void UpdateDeviceName(HWND hWnd, char* pzDevName)
{
	char szText[MAX_PATH];
	sprintf(szText,"当前设备:%s", pzDevName?pzDevName:"");
	SetDlgItemText(hWnd,IDC_TEXT_DEVICE,szText);
}

static void UpdateRecvData(HWND hWnd, BYTE* pData, DWORD dwLen)
{
	DWORD dwRemain;
	dwRecvCount += dwLen;
	UpdateDataCounter(hWnd);

	if(fpRecvFile != NULL)
	{
		fwrite(pData,1,dwLen,fpRecvFile);
	}
	
	if((bAutoClear)&&(dwRecvBuffCount>=MAX_DATA_LEN))
	{
		ClearRecvData(hWnd);
	}

	dwRemain = MAX_DATA_LEN-dwRecvBuffCount;
	dwLen = dwLen<dwRemain?dwLen:dwRemain;
	if(dwLen == 0)
	{
		return;
	}

	CopyMemory(biRecvBuff+dwRecvBuffCount, pData, dwLen);
	dwRecvBuffCount += dwLen;

	if(bHEXDisp)
	{
		BINToHEX(szRecvBuff, pData, dwLen);	
	}
	else
	{
		BINToGBK(szRecvBuff, pData, dwLen);
	}
	AppendRecvData(hWnd, szRecvBuff);
}

static DWORD WINAPI UARTRecvThread(void* arg)
{
	static char biBuff[MAX_DATA_LEN];
	DWORD dwLen;
	HWND hWnd;
	hWnd = (HWND)arg;
	while(1)
	{
		dwLen = ReadUART(hUART, biBuff, MAX_DATA_LEN);
		if(dwLen != 0)
		{
			SendMessage(hWnd,MSG_RECVDATA,dwLen, (LPARAM)biBuff);
		}
		else
		{
			Sleep(10);
		}
	}
}

static BOOL SendDataToUART(HWND hWnd)
{
	DWORD dwLen;
	DWORD dwRet;

	dwLen = GetDlgItemText(hWnd,IDC_EDIT_SEND, szSendBuff, MAX_TEXT_LEN);
	if(bHEXSend)
	{
		dwLen=HEXToBIN(szSendBuff,biSendBuff);
		dwRet=WriteUART(hUART, biSendBuff, dwLen);
	}
	else
	{
		dwRet=WriteUART(hUART, szSendBuff, dwLen);
	}
	dwSendCount += dwRet;
	UpdateDataCounter(hWnd);

	return (dwRet==dwLen);
}


static void OnBnClickedOpen(HWND hWnd)
{
	DWORD dwPID;
	int  nIndex;
	UINT nBaudrate;
	UINT nDataBits;
	UINT nParity;
	UINT nStopBits;

	if(hUART == NULL)
	{
		nIndex = (int)SendDlgItemMessage(hWnd,IDC_LIST_PORT,CB_GETCURSEL,0,0);
		if(nIndex < 0)
		{
			MessageBox(hWnd,"没有选择串口!","打开失败", MB_OK|MB_ICONERROR);
			return;
		}
		nBaudrate = GetDlgItemInt(hWnd,IDC_LIST_BAUDRATE,NULL,0);
		nDataBits = GetDlgItemInt(hWnd,IDC_LIST_DATABITS,NULL,0);
		nStopBits = (UINT)SendDlgItemMessage(hWnd,IDC_LIST_STOPBITS,CB_GETCURSEL,0,0);
		nParity   = (UINT)SendDlgItemMessage(hWnd,IDC_LIST_PARITY,CB_GETCURSEL,0,0);
		hUART = OpenUART(GetPortName(nIndex),nBaudrate,nDataBits,nStopBits,nParity);
		if(hUART == NULL)
		{
			MessageBox(hWnd,"打开串口失败!","打开失败", MB_OK|MB_ICONERROR);
			return;
		}
		hThread = CreateThread(0,0,UARTRecvThread,hWnd,0,&dwPID);
		if(hThread == NULL)
		{
			CloseUART(hUART);
			MessageBox(hWnd,"启动接收服务失败!","打开失败", MB_OK|MB_ICONERROR);
			return;
		}
		UpdateDeviceName(hWnd,GetDeviceName(nIndex));
		SetDlgItemText(hWnd,IDC_BTN_OPEN,"关闭串口(&O)");
		EnableDlgItem(hWnd,IDC_BTN_REFRESH,0);
		EnableDlgItem(hWnd,IDC_LIST_PORT,0);
		EnableDlgItem(hWnd,IDC_LIST_BAUDRATE,0);
		EnableDlgItem(hWnd,IDC_LIST_DATABITS,0);
		EnableDlgItem(hWnd,IDC_LIST_STOPBITS,0);
		EnableDlgItem(hWnd,IDC_LIST_PARITY,0);
	}
	else
	{
		TerminateThread(hThread,0);
		CloseHandle(hThread);
		CloseUART(hUART);
		hUART = NULL;
		EnableDlgItem(hWnd,IDC_BTN_REFRESH,1);
		EnableDlgItem(hWnd,IDC_LIST_PORT,1);
		EnableDlgItem(hWnd,IDC_LIST_BAUDRATE,1);
		EnableDlgItem(hWnd,IDC_LIST_DATABITS,1);
		EnableDlgItem(hWnd,IDC_LIST_STOPBITS,1);
		EnableDlgItem(hWnd,IDC_LIST_PARITY,1);
		SetDlgItemText(hWnd,IDC_BTN_OPEN,"打开串口(&O)");
		UpdateDeviceName(hWnd,NULL);
	}
}

static void OnBnClickedSend(HWND hWnd)
{
	BOOL bRet;
	bRet=SendDataToUART(hWnd);
	if(!bRet)
	{
		MessageBox(hWnd,"无法写入数据!","发送失败",MB_OK);
	}
}

static void OnBnClickedClear(HWND hWnd)
{
	ClearRecvData(hWnd);
}

static void OnBnClickedStore(HWND hWnd)
{
	char szFileName[MAX_PATH];
	BOOL bRet;
	FILE* fp;
	bRet=SaveFileBox(hWnd,"保存数据","所有文件(*.*)", szFileName);
	if(bRet)
	{
		fp =fopen(szFileName,"wb+");
		if(fp == NULL)
		{
			MessageBox(hWnd,"创建文件失败!","保存失败",MB_OK);
			return;
		}
		fwrite(biRecvBuff, 1, dwRecvBuffCount, fp);
		fclose(fp);
	}
}

static void OnBnClickedZero(HWND hWnd)
{
	dwSendCount = 0;
	dwRecvCount = 0;
	UpdateDataCounter(hWnd);
}

static void OnBnClickedHEXSend(HWND hWnd)
{
	bHEXSend = IsDlgButtonChecked(hWnd,IDC_BTN_SENDHEX);
}

static void OnBnClickedHEXDisplay(HWND hWnd)
{
	bHEXDisp = IsDlgButtonChecked(hWnd,IDC_BTN_RECVHEX);
}

static void OnBnClickedAutoClear(HWND hWnd)
{
	bAutoClear=IsDlgButtonChecked(hWnd,IDC_BTN_AUTOCLEAR);
}

static void OnBnClickedAutoSend(HWND hWnd)
{
	int nInterval;
	bAutoSend=IsDlgButtonChecked(hWnd,IDC_BTN_AUTOSEND);
	EnableDlgItem(hWnd,IDC_EDIT_INTERVAL,!bAutoSend);
	EnableDlgItem(hWnd,IDC_BTN_SEND,!bAutoSend);
	EnableDlgItem(hWnd,IDC_BTN_SENDFILE,!bAutoSend);
	if(bAutoSend)
	{
		nInterval = GetDlgItemInt(hWnd,IDC_EDIT_INTERVAL,NULL,0);
		SetTimer(hWnd,1,nInterval,NULL);
	}
	else
	{
		KillTimer(hWnd,1);
	}
}

static void OnBnClickedRefresh(HWND hWnd)
{
	FillSerialPort(GetDlgItem(hWnd,IDC_LIST_PORT));
}

static void OnBnClickedRecvFile(HWND hWnd)
{
	char szFileName[MAX_PATH];
	bRecvFile = IsDlgButtonChecked(hWnd,IDC_BTN_RECVFILE);
	if(bRecvFile)
	{
		if(SaveFileBox(hWnd,"写入文件","所有文件(*.*)", szFileName))
		{
			fpRecvFile = fopen(szFileName,"wb+");
			if(fpRecvFile == NULL)
			{
				MessageBox(hWnd,"无法创建文件!","写入失败",MB_OK|MB_ICONERROR);
				CheckDlgButton(hWnd,IDC_BTN_RECVFILE,0);
			}
		}
		else
		{
			CheckDlgButton(hWnd,IDC_BTN_RECVFILE,0);
		}
	}
	else
	{
		if(fpRecvFile != NULL)
		{
			fclose(fpRecvFile);
			fpRecvFile = NULL;
		}
	}
}

static void OnBnClickedSendFile(HWND hWnd)
{
	char szFileName[MAX_PATH];
	if(OpenFileBox(hWnd,"发送文件","所有文件(*.*)", szFileName))
	{
		PopupSendBox(hInst,hWnd,hUART,szFileName);
	}
}

static void OnTimer(HWND hWnd, WPARAM wParam)
{
	BOOL bRet;
	bRet=SendDataToUART(hWnd);
	if(!bRet)
	{
		CheckDlgButton(hWnd,IDC_BTN_AUTOSEND,0);
		OnBnClickedAutoSend(hWnd);
		MessageBox(hWnd,"无法发送数据,操作已取消!","自动发送失败",MB_OK);
	}
}

//控件消息
static void OnCommand(HWND hWnd, UINT id, UINT event)
{
	switch(id)
	{
	case IDCANCEL:
		EndDialog(hWnd,0);
		break;
	case IDC_BTN_OPEN:
		OnBnClickedOpen(hWnd);
		break;
	case IDC_BTN_SEND:
		OnBnClickedSend(hWnd);
		break;
	case IDC_BTN_ZERO:
		OnBnClickedZero(hWnd);
		break;
	case IDC_BTN_AUTOCLEAR:
		OnBnClickedAutoClear(hWnd);
		break;
	case IDC_BTN_CLEAR:
		OnBnClickedClear(hWnd);
		break;
	case IDC_BTN_RECVHEX:
		OnBnClickedHEXDisplay(hWnd);
		break;
	case IDC_BTN_WRITEFILE:
		OnBnClickedStore(hWnd);
		break;
	case IDC_BTN_AUTOSEND:
		OnBnClickedAutoSend(hWnd);
		break;
	case IDC_BTN_SENDHEX:
		OnBnClickedHEXSend(hWnd);
		break;
	case IDC_BTN_REFRESH:
		OnBnClickedRefresh(hWnd);
		break;
	case IDC_BTN_RECVFILE:
		OnBnClickedRecvFile(hWnd);
		break;
	case IDC_BTN_SENDFILE:
		OnBnClickedSendFile(hWnd);
		break;
	}
}

static void OnInitDialog(HWND hWnd)
{
	HICON hIcon;
	bInit = TRUE;
	hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_SERIALTOOL));
	SendMessage(hWnd,WM_SETICON,1,(LPARAM)hIcon);
	SetWindowText(hWnd,"串口调试工具");
	GetClientRect(hWnd,&CurRect);
	GetWindowRect(hWnd,&MinRect);

	FillSerialPort(GetDlgItem(hWnd,IDC_LIST_PORT));
	FillBaudRate(GetDlgItem(hWnd,IDC_LIST_BAUDRATE));
	FillDataBits(GetDlgItem(hWnd,IDC_LIST_DATABITS));
	FillStopBits(GetDlgItem(hWnd,IDC_LIST_STOPBITS));
	FillParityBit(GetDlgItem(hWnd,IDC_LIST_PARITY));
	SetDlgItemInt(hWnd,IDC_EDIT_INTERVAL,1000,0);
	SendDlgItemMessage(hWnd,IDC_EDIT_RECV,EM_SETLIMITTEXT,MAX_TEXT_LEN,0);
}

static void OnSize(HWND hWnd, WPARAM wParam, int nWidth, int nHeight)
{
	int  dx;
	int  dy;
	dx = nWidth-CurRect.right;
	dy = nHeight-CurRect.bottom;
	CurRect.right = nWidth;
	CurRect.bottom= nHeight;

	LockWindowUpdate(hWnd);

	//1.
	MoveDlgItem(hWnd,IDC_EDIT_RECV,dx,dy,MF_RESIZEHEIGHT|MF_RESIZEWIDTH);

	//2.
	MoveDlgItem(hWnd,IDC_EDIT_SEND,dx,dy,MF_RESIZEWIDTH|MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_TEXT_DEVICE,dx,dy,MF_RESIZEWIDTH|MF_OFFSETVER);

	//3.
	MoveDlgItem(hWnd,IDC_TEXT_RECVCOUNT,dx,dy,MF_OFFSETHOR|MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_TEXT_SENDCOUNT,dx,dy,MF_OFFSETHOR|MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_ZERO,dx,dy,MF_OFFSETHOR|MF_OFFSETVER);

	//4.
	MoveDlgItem(hWnd,IDC_GROUP_TX,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_SEND,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_SENDFILE,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_SENDHEX,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_AUTOSEND,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_BTN_LOADFILE,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_EDIT_INTERVAL,dx,dy,MF_OFFSETVER);
	MoveDlgItem(hWnd,IDC_TEXT_INTERVAL,dx,dy,MF_OFFSETVER);
	
	LockWindowUpdate(0);
}

static void OnGetMinMaxInfo(HWND hWnd, MINMAXINFO* pMMI)
{
	pMMI->ptMinTrackSize.x = MinRect.right - MinRect.left;
	pMMI->ptMinTrackSize.y = MinRect.bottom - MinRect.top;
}

INT_PTR WINAPI MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		OnInitDialog(hWnd);
		break;
	case WM_HELP:
		PopupAboutBox(hInst,hWnd);
		break;
	case WM_SIZE:
		if((wParam == SIZE_RESTORED)||(wParam==SIZE_MAXIMIZED))
		{
			OnSize(hWnd,wParam,LOWORD(lParam),HIWORD(lParam));
		}
		break;
	case WM_COMMAND:
		OnCommand(hWnd,LOWORD(wParam),HIWORD(wParam));
		break;
	case WM_GETMINMAXINFO:
		if(bInit)
		{
			OnGetMinMaxInfo(hWnd, (MINMAXINFO*)lParam);
			return 1;
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd,wParam);
		break;
	case MSG_RECVDATA:
		UpdateRecvData(hWnd,(BYTE*)lParam,(DWORD)wParam);
		break;
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrev, LPTSTR lpLine,int nShow)
{
	INITCOMMONCONTROLSEX icm;
	hInst = hInstance;
	icm.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icm.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icm);
	HookWindowCreate();
	DialogBox(hInst,MAKEINTRESOURCE(IDD_MAINBOX),NULL,MainProc);
	UnHookWindowCreate();
	return 0;
}
