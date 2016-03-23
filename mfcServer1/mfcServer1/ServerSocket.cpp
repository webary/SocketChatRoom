#include "stdafx.h"
#include "resource.h"
#include "ServerSocket.h"


CServerSocket::CServerSocket(void)
{
}


CServerSocket::~CServerSocket(void)
{
}

//响应一个新的连接请求
void CServerSocket::OnAccept(int nErrorCode)
{
    m_pDlg->AddClient();//转给主窗口相关函数处理
    CSocket::OnAccept(nErrorCode);
}

//响应一个关闭请求
void CServerSocket::OnClose(int nErrorCode)
{
    CSocket::OnClose(nErrorCode);
}

//响应接收到新的消息的请求
void CServerSocket::OnReceive(int nErrorCode)
{
    m_pDlg->ReceData(this);//转给主窗口相关函数处理
    CSocket::OnReceive(nErrorCode);
}
