
// mfcServer1Dlg.h : 头文件
//

#pragma once
#include "mfcServer1.h"

#include "ServerSocket.h"
#include <fstream>
#include <map>

class CServerSocket;

#define WM_NOTIFYICONMSG WM_USER+3 //托盘消息

#define MBox(s) MessageBox(s,"温馨提示")
#define MBox2(s1,s2) MessageBox(s1,s2)
#define elif else if
#define FOR(ii,start,end) for(ii=start;ii<end;++ii)

inline CString rightN(CString str, int n)
{
    return str.Right(str.GetLength() - n);
}

extern CString STR[5];
struct MyMsg {
    CString userId;
    CString pw;
    CString data;
    CString type;
    CString fromUser;
    CString toUser;
    explicit MyMsg(CString str = "") {
        load(str);
    }
    CString load(CString str) {
        CString tempStr[6] = { "" };
        int index = 0, i;
        FOR(index, 0, 5) {
            i = str.Find(STR[index]);
            tempStr[index] = str.Left(i);
            str = rightN(str, i + 3);
            if (str == "")
                break;
        }
        tempStr[5] = str;
        i = str.Find(STR[0]);
        if (i != -1)
            str = rightN(str, i + 3);
        index = 0;
        userId = tempStr[index++];
        pw = tempStr[index++];
        fromUser = tempStr[index++];
        toUser = tempStr[index++];
        type = tempStr[index++];
        data = tempStr[index++];
        return str;
    }
    const CString join(CString _data = "", CString _type = "", CString _user = "", CString _from = "", CString _to = "", CString _pw = "") const {
        if (_user == "")
            _user = userId;
        //用户名+密码+来自+去向+类型+内容
        return _user + STR[0] + _pw + STR[1] + _from + STR[2] + _to + STR[3] + _type + STR[4] + _data;
    }
};
// CmfcServer1Dlg 对话框
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
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

protected:
    HICON m_hIcon;
    NOTIFYICONDATA nd;  //通知栏图标

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
public:
    const static int UserNumMax = 1000; //最大允许有1000个用户
    struct _UserInfo {
        char User[17]; //用户名
        char Pw[17];   //密码
        bool Online;   //在线状态
        bool refuse;   //是否被拒绝登录
    } userInfo[UserNumMax];
    typedef std::map<std::string, CServerSocket*> UserSocket;
    UserSocket user_socket; //保存<用户名,socket>映射关系
    MyMsg mymsg;
    int userNum;            //所用已注册用户数
    CString fileUser;       //与文件传输相关的用户
    CString userList;       //用户列表,在用户登录时将发给用户
    CServerSocket* listenSocket;//用于服务器建立监听连接
    bool m_connect;         //用于标记服务器状态
    CString fileSendName;   //将要发送的文件的文件名
    bool readFileEnd;       //标记文件读取是否完成
    RecvFile rf;            //接收文件对象
    struct SendFileTo {
        CString fileToUser;     //服务器暂存的文件的目的用户
        CString fileFromUser;   //服务器暂存的文件的来源用户
        bool fileSendOver;      //服务器需要转发的文件是否发送完毕
        CString fileInfo;       //需要发送的文件的信息
        void set(CString to, CString from, bool over, CString info) {
            fileToUser = to;
            fileFromUser = from;
            fileSendOver = over;
            fileInfo = info;
        }
    } sft;  //服务器转发文件结构

    void AddClient();                       // 增加用户，响应用户请求
    void RemoveClient(const CString _user); // 移除下线的用户
    void ReceData(CServerSocket* pSocket);  // 获取数据
    void UpdateEvent(CString str, CString from = "服务器\t");// 更新事件日志
    void SendMSG(CString str);              // 发送消息给各个客户端
    void ControlPC(CString AndroidControl); // 手机控制PC的响应函数
    void SendCloseMsg();  // 发送关闭服务器的消息
    int UserInfoValid(bool CheckOnline = 0, bool onlyUser = 0, CString checkUser = "");  // 验证用户名和密码是否有效
    int GetOnlineNum();   //得到在线用户数目
    bool isOnline(CString _user);   //得到用户是否在线，返回序号
    void fileSend(MyMsg& msg, bool NoAsk = 0);       //收到用户from发来文件的请求
    int fileTransfer(MyMsg& msg, const char* pData); //文件传输
    void modifyStatus(CString sta, bool _sleep = 1); //修改状态栏
    void sendFileToOthers(bool first = 0);  //继续发送文件给其他用户

    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

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
};
