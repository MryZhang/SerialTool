#ifndef __CONVERT_H
#define __CONVERT_H

//字符转为数据，返回实际长度
//示意:"1234" -> 0x12 0x34
DWORD HEXToBIN(CHAR* pHEX, BYTE* pBIN);

//数据转换为字符串
//示意:0x12 0x34 -> "12 34"
void BINToHEX(CHAR* pzHEX, BYTE* pBIN, DWORD dwLen);

//解决半个汉字的问题
void BINToGBK(char* pzGBK, BYTE* pBIN, DWORD dwLen);

#endif
