#include "stdafx.h"
#include "convert.h"

//十六进制转数据，返回数据长度
//示意:"12 34" -> 0x12 0x34
DWORD HEXToBIN(CHAR* pHEX, BYTE* pBIN)
{
	DWORD dwLen;
	CHAR  ch;
	BYTE  biFlag;
	BYTE  biHalfByte;

	biFlag = 0;
	biHalfByte = 0;
	
	for(dwLen=0;ch=*pHEX;pHEX++)
	{
		if ((ch>='0')&&(ch<='9'))
		{
			biHalfByte = ch - '0';
		}
		else if((ch>='A')&&(ch<='F'))
		{
			biHalfByte = ch - 'A' + 10;
		}
		else if((ch>='a')&&(ch<='f'))
		{
			biHalfByte = ch - 'a' + 10;
		}
		else
		{
			continue;
		}
		if(biFlag==0)
		{
			*pBIN = biHalfByte<<4;
		}
		else
		{
			*pBIN |= biHalfByte;
			pBIN++;
			dwLen++;
		}
		biFlag = !biFlag;
	}
	return dwLen;
}


//二进制转换为十六进制
//示意:0x12 0x34 -> "12 34"
static CONST CHAR chCode[]={"0123456789ABCDEF"};
void BINToHEX(CHAR* pzHEX, BYTE* pBIN, DWORD dwLen)
{
	DWORD i;

	for(i=0; i<dwLen; i++)
	{
		*pzHEX++ = chCode[(*pBIN>>4)&0x0F];
		*pzHEX++ = chCode[(*pBIN)&0x0F];
		*pzHEX++ = ' ';
		pBIN++;
	}
	*pzHEX = 0;
}

//解决半个汉字的问题
void BINToGBK(char* pzGBK, BYTE* pBIN, DWORD dwLen)
{
	static BOOL bHalf;
	static char chHalf;
	DWORD i;

	if(bHalf)
	{
		if(*pBIN > 0x80)
			*pzGBK++ = chHalf;
		else
			bHalf = 0;
	}

	for(i=0;i<dwLen;i++)
	{
		if(*pBIN > 0x80)
		{
			bHalf = !bHalf;
		}
		*pzGBK++ = *pBIN++;
	}

	if(bHalf)
	{
		if(*(pBIN-1)>0x80)
		{
			pzGBK--;
			chHalf = *pzGBK;
		}
		else
		{
			bHalf = 0;
		}
	}
	*pzGBK = 0;
}
