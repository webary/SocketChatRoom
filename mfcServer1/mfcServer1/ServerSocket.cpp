#include "stdafx.h"
#include "ServerSocket.h"
#include "mfcServer1Dlg.h"

CServerSocket::CServerSocket(CmfcServer1Dlg* pDlg) : m_pDlg(pDlg)
{
}

//响应一个新的连接请求
void CServerSocket::OnAccept(int nErrorCode)
{
    m_pDlg->addClient();//转给主窗口相关函数处理
    CSocket::OnAccept(nErrorCode);
}

//响应接收到新的消息的请求
void CServerSocket::OnReceive(int nErrorCode)
{
    m_pDlg->receData(this);//转给主窗口相关函数处理
    CSocket::OnReceive(nErrorCode);
}
