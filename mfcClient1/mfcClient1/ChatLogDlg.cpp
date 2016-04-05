#include "stdafx.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"
#include "ChatLogDlg.h"

#include <string>
using namespace std;

BEGIN_MESSAGE_MAP(CChatLogDlg, CDialogEx)
    ON_BN_CLICKED(IDC_DelAllLog, &CChatLogDlg::OnDelAllLog)
    ON_BN_CLICKED(IDC_Update, &CChatLogDlg::OnUpdateLog)
END_MESSAGE_MAP()

void CChatLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CChatLogDlg::OnInitDialog()
{
    p_editCL = (CEdit*)GetDlgItem(IDC_Chatlog);
    //加载该用户的聊天记录
    chatLog = "";
    string _str;
    ifstream tmpF(ClientInfo::myDIR + userID, ios::in);
    while (tmpF.is_open() && !tmpF.eof()) {
        getline(tmpF, _str);
        if (_str != "") {
            chatLog += _str.c_str();
            chatLog += "\r\n";
        }
    }
    tmpF.close();
    if (chatLog == "")
        chatLog = "暂时无任何聊天记录";
    return TRUE;
}

void CChatLogDlg::OnDelAllLog()
{
    if (chatLog != "暂时无任何聊天记录"
        && MessageBox("***确认删除所有聊天记录吗？此操作不可撤回！***", "删除确认", MB_YESNO) == IDYES) {
        DeleteFile(ClientInfo::myDIR + userID);
        MessageBox("已删除所有聊天记录！", "温馨提示");
    } else if (chatLog == "暂时无任何聊天记录") {
        MessageBox("暂时无任何聊天记录", "温馨提示");
    } else {
        return;
    }
    ::SendMessage(GetSafeHwnd(), WM_CLOSE, 0, 0);
}

void CChatLogDlg::OnUpdateLog()
{
    chatLog = "";
    string _str;
    ifstream tmpF(ClientInfo::myDIR + userID, ios::in);
    while (!tmpF.eof() && tmpF.is_open()) {
        getline(tmpF, _str);
        if (_str != "")
            chatLog += (_str+"\r\n").c_str();
    }
    tmpF.close();
    if (chatLog == "")
        chatLog = "暂无任何聊天记录";
    SetDlgItemText(IDC_Chatlog, "");
    int lastLine = p_editCL->LineIndex(p_editCL->GetLineCount() - 1);
    p_editCL->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
    p_editCL->ReplaceSel(chatLog);   //替换所选那一行的内容
}
