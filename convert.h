#ifndef __CONVERT_H
#define __CONVERT_H

//�ַ�תΪ���ݣ�����ʵ�ʳ���
//ʾ��:"1234" -> 0x12 0x34
DWORD HEXToBIN(CHAR* pHEX, BYTE* pBIN);

//����ת��Ϊ�ַ���
//ʾ��:0x12 0x34 -> "12 34"
void BINToHEX(CHAR* pzHEX, BYTE* pBIN, DWORD dwLen);

//���������ֵ�����
void BINToGBK(char* pzGBK, BYTE* pBIN, DWORD dwLen);

#endif
