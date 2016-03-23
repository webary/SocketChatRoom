
// mfcClient1Dlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ClientSocket.h"
#include "afxcmn.h"


// CLoginDlg (登陆)对话框
class CLoginDlg : public CDialogEx
{
public:
    CLoginDlg() : CDialogEx(CLoginDlg::IDD), userID(_T("")), pw(_T("")), port(22783) {}
    // 对话框数据
    enum { IDD = IDD_LOGIN };
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    virtual BOOL OnInitDialog();
public:
    bool b_strech;	//是否已展开扩展区
    CRect m_rc;
    CString userID, pw;
    CIPAddressCtrl m_IPAddr;
    CString ip;
    int port;

    bool onlyAlNum(const CString s);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedSet();
    afx_msg void OnBnClickedregist();
    bool DataInvalide();
    afx_msg void OnBnClickedCheckrempw();
    afx_msg void OnBnClickedSwap();
    afx_msg void OnBnClickedShowpw();
};

extern CString myDIR;		//全局外部变量，个人文件夹，位于临时目录
class CmfcClient1Dlg;
class CClientDlg
{
public:
    CString m_userID,m_pw;	//用户名,密码
    int userNum;			//已注册用户个数
    CString m_DataSend;		//将要发送的消息
    CString m_msgTo;		//消息的目的用户
    bool to_isOnline;		//目的用户是否在线？

    int m_port;
    CString m_ip;
    CLoginDlg login;		//连接的主对话框
    bool logining;			//当前正在登陆？
    bool m_connect;			//当前已连接到服务器？
    bool firstCon;			//第一次连接？

    RecvFile rf;			//接收文件对象
    CString fileUser;		//与文件传输关联的用户
    CString fileSendName;	//要发送的文件的完整文件名
    int fileTimeOut;		//文件超时时长
    bool readFileEnd;		//读取文件结束？

    CString myDIR;			//个人文件夹，位于临时目录
    CClientSocket* pSock;   //客户端套接字指针对象
    CmfcClient1Dlg* pDlg;	//主窗口指针
public:
    CClientDlg() {
        pDlg = 0;
        firstCon = 1;
        logining = 0;
        readFileEnd = 0;
        m_connect = 0;
        m_DataSend = m_userID = "";

        char path[2048] = "";
        GetTempPath(2048,path);
        strcat(path,"mfcClient1");
        CreateDirectory(path,0);
        AfxGetApp()->WriteProfileString("ClientSetting", "tempDir", path);
        myDIR = path + CString("\\");
        ::myDIR = myDIR;
        DeleteFile(myDIR+"send.txt");
        DeleteFile(myDIR+"TransLog.txt");
    }
    virtual ~CClientDlg() {
        if (m_connect) {
            pSock->SendMSG( pSock->mymsg.join("",TYPE[Logout],"","","",m_pw) ,0);
            pSock->Close();
            delete pSock;
        }
    }
    //保存文件内容并发送文件属性
    void save_SendFileInfo(const char* filepath,long size);
    //登录对话框
    int  loginDlg();
    //修改状态栏
    void modifyStatus(CString sta,bool _sleep=1);
};

//消息记录
class CChatLogDlg : public CDialogEx
{
public:
    CChatLogDlg(CString id) : CDialogEx(CChatLogDlg::IDD) {
        userID = id;
    }
    // 对话框数据
    enum { IDD = IDD_ChatLog };
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    virtual BOOL OnInitDialog();
public:
    CEdit* p_editCL;
    CString userID,chatLog;
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedDelalllog();
    afx_msg void OnBnClickedUpdate();
};

// CmfcClient1Dlg 对话框
class CmfcClient1Dlg : public CDialogEx,public CClientDlg
{
public:
    CmfcClient1Dlg(CWnd* pParent = NULL): CClientDlg(),CDialogEx(CmfcClient1Dlg::IDD, pParent) {
        pChatlog = 0;
    }
    ~CmfcClient1Dlg();
// 对话框数据
    enum { IDD = IDD_MFCCLIENT1_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    virtual void OnOK();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

// 实现
public:
    HICON m_hIcon;
    CChatLogDlg* pChatlog;
    CButton m_ConPC;		//连接服务器按钮
    NOTIFYICONDATA nd;		//通知栏图标
    CComboBox m_cbMsgTo;	//发送给下拉框

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedConnect();
    afx_msg void OnBnClickedSend();
    afx_msg void OnBnClickedLogoff();
    afx_msg void OnBnClickedChatlog();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnCbnSelChangeMsgTo();
    afx_msg void OnDropFiles(HDROP hDropInfo);
};
