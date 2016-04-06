#pragma once
#ifndef _CLIENTSOCKET_H_
#define _CLIENTSOCKET_H_

#include <afxsock.h>

#define DATA_BUF_SIZE   (32*1024) //数据缓冲区大小
#define PACKAGE_SIZE    (512)     //文件拆包后每个包最大字节数
#define MAX_PACKAGE_NUM (16*1024) //最大数据包数目

//下面是一些操作ini配置文件的宏操作
#define GETS(y,z) AfxGetApp()->GetProfileString("ClientSetting",y,z)
#define WRITE(x,y,z) { char *s = x;if(x==0) s="ClientSetting"; AfxGetApp()->WriteProfileString(s,y,z);}
//x保存的整型变量，y键码，z键值的默认值
#define GET_WRITE(x,y,z) {\
	if((x=atoi(GETS(y,"-1")))==-1){\
		x = atoi(z);\
		WRITE(0,y,z);\
	}\
}

class CmfcClient1Dlg;
class CClientSocket : public CSocket
{
public:
    CClientSocket(const CString &_user);
    //重写接收函数，通过类向导生成
    virtual void OnReceive(int nErrorCode);
    //获取上一个socket错误提示的字符串值
    static CString getLastErrorStr();
public:
    CmfcClient1Dlg* pDlg;
};

#endif //_CLIENTSOCKET_H_
