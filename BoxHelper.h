#ifndef __BOXHELPER_H
#define __BOXHELPER_H


#define MF_RESIZEWIDTH	0x01	//�ı���
#define MF_RESIZEHEIGHT	0x02	//�ı�߶�
#define MF_OFFSETHOR	0x04	//ˮƽ�ƶ�
#define MF_OFFSETVER	0x08	//��ֱ�ƶ�
void MoveDlgItem(HWND hWnd,int nItem, int dx, int dy, UINT nFlag);

void CenterDialogBox(HWND hWnd);

void EnableDlgItem(HWND hWnd, int nItem,BOOL bEnable);

#endif
