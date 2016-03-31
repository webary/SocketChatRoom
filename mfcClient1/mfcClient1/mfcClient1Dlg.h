
// mfcClient1Dlg.h : 头文件
//

#pragma once
#include "ClientSocket.h"
#include "LoginDlg.h"
#include "ChatLogDlg.h"


extern CString myDIR;       //全局外部变量，个人文件夹，位于临时目录
class CmfcClient1Dlg;
class ClientInfo
{
public:
    CString m_userID, m_pw; //用户名,密码
    int userNum;            //已注册用户个数
    CString m_DataSend;     //将要发送的消息
    CString m_msgTo;        //消息的目的用户
    bool to_isOnline;       //目的用户是否在线？

    int m_port;
    CString m_ip;
    bool logining;          //当前正在登陆？
    bool m_connect;         //当前已连接到服务器？
    bool firstCon;          //第一次连接？

    RecvFile rf;            //接收文件对象
    CString fileUser;       //与文件传输关联的用户
    CString fileSendName;   //要发送的文件的完整文件名
    int fileTimeOut;        //文件超时时长
    bool readFileEnd;       //读取文件结束？

    CString myDIR;          //个人文件夹，位于临时目录
    CClientSocket* pSock;   //客户端套接字对象指针
    CmfcClient1Dlg* pDlg;   //主窗口指针
public:
    ClientInfo() {
        pDlg = 0;
        firstCon = 1;
        logining = 0;
        readFileEnd = 0;
        m_connect = 0;
        m_DataSend = m_userID = "";

        char path[2048] = "";
        GetTempPath(2048, path);
        strcat(path, "mfcClient1");
        CreateDirectory(path, 0);
        AfxGetApp()->WriteProfileString("ClientSetting", "tempDir", path);
        myDIR = path + CString("\\");
        ::myDIR = myDIR;
        DeleteFile(myDIR + "send.txt");
        DeleteFile(myDIR + "TransLog.txt");
    }
    virtual ~ClientInfo() {
        if (m_connect) {
            pSock->SendMSG(pSock->mymsg.join("", TYPE[Logout], "", "", "", m_pw), 0);
            pSock->Close();
            delete pSock;
        }
    }
    //保存文件内容并发送文件属性
    void save_SendFileInfo(const char* filepath, long size);
    //显示登录对话框要求登录
    bool login();
    //修改状态栏
    void modifyStatus(CString sta, bool _sleep = 1);
};


// CmfcClient1Dlg 对话框
class CmfcClient1Dlg : public CDialogEx, public ClientInfo
{
public:
    CmfcClient1Dlg(CWnd* pParent = NULL) : ClientInfo(), CDialogEx(CmfcClient1Dlg::IDD, pParent) {
        pChatlog = 0;
    }
    ~CmfcClient1Dlg();
    // 对话框数据
    enum { IDD = IDD_MFCCLIENT1_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual void OnOK();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // 实现
public:
    HICON m_hIcon;
    CChatLogDlg* pChatlog;
    CButton m_ConPC;        //连接服务器按钮
    NOTIFYICONDATA nd;      //通知栏图标
    CComboBox m_cbMsgTo;    //发送给下拉框

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg HCURSOR OnQueryDragIcon();

    afx_msg void OnConnect();
    afx_msg void OnSend();
    afx_msg void OnLogoff();
    afx_msg void OnChatlog();
    afx_msg void OnCbnSelChangeMsgTo();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    DECLARE_MESSAGE_MAP()
};
