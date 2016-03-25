#pragma once
//数据缓冲区大小
#define DATA_BUF_SIZE		(8*1024)
//文件拆包后每个包最大字节数
#define PACKAGE_SIZE		(512)
//最大数据包数目
#define MAX_PACKAGE_NUM		(16*1024)

#include "../../mfcClient1/mfcClient1/RecvFile.hpp"

#include "mfcServer1Dlg.h"		//主对话框头文件

class CmfcServer1Dlg; //类声明,因为下面定义了一个主窗口指针
class CServerSocket : public CSocket
{
public:
    CmfcServer1Dlg* m_pDlg;
    CServerSocket(void);
    ~CServerSocket(void);
    virtual void OnAccept(int nErrorCode);
    virtual void OnClose(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
};
