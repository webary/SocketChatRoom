
// mfcServer1Dlg.h : 头文件
//

#pragma once
#include "mfcServer1.h"
#include "ServerSocket.h"
#include "../../mfcClient1/mfcClient1/MyMsg.h"
#include "../../mfcClient1/mfcClient1/RecvFile.hpp"
#include <fstream>
#include <map>

#define elif else if

#define WM_NOTIFYICONMSG WM_USER+3 //托盘消息

class CServerSocket;
class CmfcServer1Dlg : public CDialogEx
{
#define DATASRC CString("UserData.dat")
#define OLMSG CString("offlineMsg")
public:
    CmfcServer1Dlg(CWnd* pParent = NULL);   // 标准构造函数
    ~CmfcServer1Dlg();

    // 对话框数据
    enum { IDD = IDD_MFCSERVER1_DIALOG };

protected:
    HICON m_hIcon;

    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
private:
    const static int UserNumMax = 100; //允许登录的用户最大数量
    struct _UserInfo {
        char userName[17];      //用户名
        char pwd[17];           //密码
        bool isOnline;          //在线状态
        bool isRefused;         //是否被拒绝登录
    } userInfo[UserNumMax];
    typedef std::map<std::string, CServerSocket*> UserSocket;
    UserSocket user_socket;     //保存<用户名,socket>映射关系
    int userNum;                //所有已注册用户数
    CString userList;           //用户列表,在用户登录时将发给用户
    CServerSocket* listenSocket;//用于服务器建立监听连接
    bool m_connect;             //用于标记服务器状态
    CString fileSendName;       //将要发送的文件的文件名
    bool readFileEnd;           //标记文件读取是否完成
    RecvFile rf;                //接收文件对象
    struct SendFileTo {
        CString fileToUser;     //服务器暂存的文件的目的用户
        CString fileFromUser;   //服务器暂存的文件的来源用户
        bool fileSendOver;      //服务器需要转发的文件是否发送完毕
        CString fileInfo;       //需要发送的文件的信息
        void set(const CString &to, const CString &from, bool over, const CString &info) {
            fileToUser = to;
            fileFromUser = from;
            fileSendOver = over;
            fileInfo = info;
        }
    } sft;              //服务器转发文件的结构
    NOTIFYICONDATA nd;  //通知栏图标

public:                 //这两个成员在类外部也需要访问，故设置为公有
    MyMsg mymsg;
    CString fileUser;   //与文件传输相关的用户

public:
    void addClient();                       // 增加用户，响应用户请求
    void removeClient(const CString _user); // 移除下线的用户
    void receData(CServerSocket* pSocket);  // 获取数据
    void updateEvent(CString str, CString from = "服务器\t");// 更新事件日志
    void sendMSG(CString str);              // 发送消息给各个客户端
    void controlPC(CString AndroidControl); // 手机控制PC的响应函数
    void sendCloseMsg();  // 发送关闭服务器的消息
    int isUserInfoValid(bool CheckOnline = 0, bool onlyUser = 0, CString checkUser = "");  // 验证用户名和密码是否有效
    int getOnlineUserNums();   //得到在线用户数目
    bool isUserOnline(CString _user);   //得到用户是否在线，返回序号
    void fileSend(MyMsg& msg, bool NoAsk = 0);       //收到用户from发来文件的请求
    int fileTransfer(MyMsg& msg, const char* pData); //文件传输
    void modifyStatus(CString sta, bool _sleep = 1); //修改状态栏
    void sendFileToOthers(bool first = 0);  //继续发送文件给其他用户

    UINT m_port;
    CEdit m_event;
    UINT m_userOnlineCount;
    DWORD m_ip;
    afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam, LPARAM lParam);//处理通知栏消息
    afx_msg void OnOpenCloseServer();
    afx_msg void OnHide();
    afx_msg void OnExit();
    afx_msg void OnShow();
    afx_msg void OnClose();
    afx_msg void OnClearLog();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    DECLARE_MESSAGE_MAP()
};
