#include "stdafx.h"
#include "ServerSocket.h"
#include "mfcServer1Dlg.h"

CString TYPE[30] = { TYPE_ChatMsg , TYPE_Server_is_closed , TYPE_UserList , TYPE_OnlineState , TYPE_FileSend , TYPE_FileData , TYPE_AskFileData , TYPE_File_NO , TYPE_File_Over , TYPE_File_Fail , TYPE_LoginFail , TYPE_UserIsOnline , TYPE_OfflineMsg , TYPE_AllUser , TYPE_AddUserList , TYPE_I_am_online , TYPE_Logout , TYPE_Login , TYPE_Register , TYPE_Status };
//用户名+密码+来自+去向+类型+内容
CString STR[5] = { "@@@","<<<",">>>","&&&","###" };

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
