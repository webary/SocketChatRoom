#include "stdafx.h"
#include "ClientSocket.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"


CString TYPE[30] = { TYPE_ChatMsg , TYPE_Server_is_closed , TYPE_UserList , TYPE_OnlineState , TYPE_FileSend , TYPE_FileData , TYPE_AskFileData , TYPE_File_NO , TYPE_File_Over , TYPE_File_Fail , TYPE_LoginFail , TYPE_UserIsOnline , TYPE_OfflineMsg , TYPE_AllUser , TYPE_AddUserList , TYPE_I_am_online , TYPE_Logout , TYPE_Login , TYPE_Register , TYPE_Status };
//用户名+密码+来自+去向+类型+内容
CString STR[5] = { "@@@","<<<",">>>","&&&","###" };

CClientSocket::CClientSocket(const CString &_user)
{
    pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
}

//	说明：当客户端接收到服务器端发的数据时会响应接收函数OnReceive
void CClientSocket::OnReceive(int nErrorCode)
{
    pDlg->receData();
    CSocket::OnReceive(nErrorCode);
}

CString CClientSocket::getLastErrorStr()
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
        , 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
        , (LPTSTR)&lpMsgBuf, 0, 0);
    CString errStr = (LPCTSTR)lpMsgBuf;
    LocalFree(lpMsgBuf);
    return errStr;
}
