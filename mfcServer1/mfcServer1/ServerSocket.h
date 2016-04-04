#pragma once
#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#define DATA_BUF_SIZE		(8*1024)    //数据缓冲区大小
#define PACKAGE_SIZE		(512)       //文件拆包后每个包最大字节数
#define MAX_PACKAGE_NUM		(16*1024)   //最大数据包数目

#include "../../mfcClient1/mfcClient1/RecvFile.hpp"

#include "mfcServer1Dlg.h"		//主对话框头文件

class CmfcServer1Dlg; //类声明,因为下面定义了一个主窗口指针
class CServerSocket : public CSocket
{
public:
    CServerSocket(const CmfcServer1Dlg* m_pDlg);
    virtual void OnAccept(int nErrorCode);
    virtual void OnClose(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
public:
    CmfcServer1Dlg* m_pDlg;
};

#endif //_SERVERSOCKET_H_
