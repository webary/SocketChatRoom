
// mfcClient1Dlg.h : 头文件
//

#pragma once
#include "ClientSocket.h"
#include "LoginDlg.h"
#include "ChatLogDlg.h"

#include "MyMsg.h"
#include "RecvFile.hpp"

#define elif else if
#define MBox(s) MessageBox(s,"温馨提示")

class CmfcClient1Dlg;
class ClientInfo
{
public:
    ClientInfo();
public:
    CString m_userID, m_pw; //用户名,密码
    bool m_connected;       //当前已连接到服务器？
    static int userNum;     //已注册用户个数

    CString m_DataSend;     //将要发送的消息
    CString m_msgTo;        //消息的目的用户
    bool m_toUserIsOnline;  //目的用户是否在线？

    CString m_ip;           //连接到的ip地址
    int m_port;             //连接到的端口号

    RecvFile m_rf;          //用于接收文件的对象
    CString m_fileUser;     //与文件传输关联的用户
    CString m_fileSendName; //要发送的文件的完整文件名
    int m_fileTimeOut;      //文件传输超时时长，毫秒
    bool m_readFileEnd;     //读取文件结束？

    CClientSocket* pSock;   //客户端套接字对象指针
    CmfcClient1Dlg* pDlg;   //主窗口指针

    static CString myDIR;   //个人文件夹，位于临时目录
};


// CmfcClient1Dlg 对话框
class CmfcClient1Dlg : public CDialogEx, public ClientInfo
{
public:
    CmfcClient1Dlg(CWnd* pParent = NULL);
    ~CmfcClient1Dlg();

    enum {
        IDD = IDD_MFCCLIENT1_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 支持
    virtual void OnOK();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();

public:
    HICON m_hIcon;
    CChatLogDlg* pChatlog;
    CButton m_ConPC;        //连接服务器按钮
    NOTIFYICONDATA nd;      //通知栏图标
    CComboBox m_cbMsgTo;    //发送给下拉框
    MyMsg mymsg;
    bool autoConnect;       //是否在执行自动定时连接服务器的过程
public:
    //接收到消息，需要处理
    void receData();
    //发送函数，用于发送数据给服务器
    void sendMSG(const CString &send, bool upEvent = 1);
    //更新消息面板
    void updateEvent(const CString &showMsg, const CString &from = "服务器:",
                     bool reset = 0, int timeFMT = 2);
    //接收到发送文件的请求时
    void fileSend(MyMsg& msg);
    //修改状态栏
    void modifyStatus(const CString &sta, bool _sleep = 1);
    //保存文件内容并发送文件属性
    void save_SendFileInfo(const CString &filepath, long size);
    //弹出登录对话框要求登录
    bool login();

    // 消息映射函数
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    //定时器响应函数
    afx_msg void OnTimer(UINT nIDEvent);
    //连接服务器按钮响应函数
    afx_msg void OnConnect();
    //发送按钮响应函数
    afx_msg void OnSend();
    //注销按钮响应函数
    afx_msg void OnLogoff();
    //聊天记录按钮响应函数
    afx_msg void OnChatlog();
    //改变目标用户，改选下拉框
    afx_msg void OnCbnSelChangeMsgTo();
    //拖入了文件
    afx_msg void OnDropFiles(HDROP hDropInfo);
    DECLARE_MESSAGE_MAP()
};
