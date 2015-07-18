#include "stdafx.h"
#include "resource.h"
#include "ServerSocket.h"


CServerSocket::CServerSocket(void)
{
}


CServerSocket::~CServerSocket(void)
{
}


void CServerSocket::OnAccept(int nErrorCode)
{
    m_pDlg->AddClient();    //添加上线用户
	CSocket::OnAccept(nErrorCode);
}


void CServerSocket::OnClose(int nErrorCode)
{
	CSocket::OnClose(nErrorCode);
}


void CServerSocket::OnReceive(int nErrorCode)
{
	m_pDlg->ReceData(this);     // 接收数据
	CSocket::OnReceive(nErrorCode);
}
