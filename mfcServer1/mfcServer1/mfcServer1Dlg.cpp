
// mfcServer1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfcServer1.h"
#include "mfcServer1Dlg.h"
#include <string>

#include "../../mfcClient1/mfcClient1/md5.h"
#include "../../mfcClient1/mfcClient1/CXXFStream.hpp"

using namespace std;

char packageData[MAX_PACKAGE_NUM][2 * PACKAGE_SIZE + 1];//保存需要发送的文件内容

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() : CDialogEx(CAboutDlg::IDD)
    { }

    enum { IDD = IDD_ABOUTBOX };
};


// CmfcServer1Dlg 对话框

CmfcServer1Dlg::CmfcServer1Dlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CmfcServer1Dlg::IDD, pParent)
    , m_port(22783)
{
    m_connect = 0;
    //将用户信息读入用户表
    fstream file(DATASRC, ios::out | ios::_Noreplace);
    file.close();
    CreateDirectory(OLMSG, 0);
    fstream src(DATASRC, ios::in);
    userNum = 0;
    userList = "";
    while (!src.eof()) {
        src >> userInfo[userNum].userName >> userInfo[userNum].pwd;
        if (0 == userInfo[userNum].userName[0] || 0 == userInfo[userNum].pwd[0])
            break;
        userInfo[userNum].isOnline = userInfo[userNum].isRefused = 0;
        userList += userInfo[userNum].userName;
        userList += ";";
        CStdioFile(OLMSG + "//" + CString(userInfo[userNum].userName), CFile::modeCreate | CFile::modeNoTruncate);
        CreateFile(userInfo[userNum].userName, 0, 0, 0, 0, 0, 0);
        userNum++;
    }
    src.close();
    getOnlineUserNums();
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);//(IDR_MAINFRAME);
    DeleteFile("ServerMsg.txt");
    sft.fileSendOver = 1;	//默认没有需要转发的文件
}

CmfcServer1Dlg::~CmfcServer1Dlg()
{
    Shell_NotifyIcon(NIM_DELETE, &nd);
}
void CmfcServer1Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PortId, m_port);
    DDX_Text(pDX, IDC_userC, m_userOnlineCount);
    DDX_Control(pDX, IDC_EDIT2, m_event);
}

BEGIN_MESSAGE_MAP(CmfcServer1Dlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_WM_DROPFILES()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_OpenCloseServer, OnOpenCloseServer)
    ON_MESSAGE(WM_NOTIFYICONMSG, OnNotifyIconMsg)
    ON_COMMAND(ID_hide, OnHide)
    ON_COMMAND(ID_exit, OnExit)
    ON_COMMAND(ID_show, OnShow)
END_MESSAGE_MAP()

//获得外网IP,在本项目发布版本中暂未用到
#include <afxinet.h>
void GetOutIP()
{
    CInternetSession session;
    CInternetFile * httpfile = (CInternetFile *)session.OpenURL("http://1111.ip138.com/ic.asp");
    CString content;
    CString Data;      //存放返回的网页数据
    while (httpfile->ReadString(content)) {
        Data += content;
    }
    int start = Data.Find("[");
    int end = Data.Find("]");
    int count = end - start;
    Data = Data.Mid(start + 1, count - 1);
    MessageBox(0, "本机ip: " + Data, "", 0);
    httpfile->Close();
    session.Close();
}

// CmfcServer1Dlg 消息处理程序

BOOL CmfcServer1Dlg::OnInitDialog()
{
    //利用互斥锁机制保证最多只有一个服务器程序正在运行
    HANDLE hmutex = ::CreateMutex(NULL, true, "myMFCServer1");	//创建互斥对象并返回其句柄
    if (hmutex) {
        if (ERROR_ALREADY_EXISTS == GetLastError()) {//若已存在
            SetWindowText("NewWindwos");//设置当前窗口标题
            HWND appWnd = ::FindWindow(0, "mfcServer1");//利用标题查找窗口
            if (appWnd != NULL) { //如果找到已在运行的服务器程序窗口则激活显示
                ::ShowWindow(appWnd, SW_SHOWNORMAL);
                ::SetForegroundWindow(appWnd);
            }
            exit(0);
        }
    }
    else
        exit(0);

    CDialogEx::OnInitDialog();

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        pSysMenu->AppendMenu(MF_SEPARATOR);
        pSysMenu->AppendMenu(MF_STRING, ID__clear, "清空记录");
        /*
        // 将“关于...”菜单项添加到系统菜单中。
        // IDM_ABOUTBOX 必须在系统命令范围内。

        ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
        ASSERT(IDM_ABOUTBOX < 0xF000);
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
        */
    }

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    LONG style = ::GetWindowLong(this->GetSafeHwnd(), GWL_STYLE);
    style &= ~WS_THICKFRAME;//使窗口不能用鼠标改变大小
    ::SetWindowLong(this->GetSafeHwnd(), GWL_STYLE, style);
    //添加状态栏
    HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, "欢迎使用本软件!", this->m_hWnd, 0);
    ::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 120, 200));
    //添加托盘图标
    nd.cbSize = sizeof(NOTIFYICONDATA);
    nd.hWnd = m_hWnd;
    nd.uID = IDI_ICON1;
    nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nd.uCallbackMessage = WM_NOTIFYICONMSG;
    nd.hIcon = m_hIcon;
    sprintf_s(nd.szTip, "服务器");
    Shell_NotifyIcon(NIM_ADD, &nd);

    OnOpenCloseServer();
    SetForegroundWindow(); //设置窗口激活为最前端显示
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmfcServer1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else if (nID == ID__clear) {
        OnClearLog();
    }
    else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CmfcServer1Dlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CmfcServer1Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

//确认按钮的响应函数
void CmfcServer1Dlg::OnOK()
{
    OnOpenCloseServer();
}

//处理托盘消息
LRESULT CmfcServer1Dlg::OnNotifyIconMsg(WPARAM wParam, LPARAM lParam)
{
    CPoint Point;
    switch (lParam) {
        case WM_RBUTTONDOWN:
        { //如果按下鼠标右建
            CMenu pMenu;//加载菜单
            if (pMenu.LoadMenu(IDR_MENU1)) {
                CMenu* pPopup = pMenu.GetSubMenu(0);
                GetCursorPos(&Point);
                SetForegroundWindow();
                pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, this);
            }
        }
        break;
        case WM_LBUTTONDOWN:
            this->ShowWindow(SW_SHOW);
            break;
        default:
            break;
    }
    return 0;
}

void CmfcServer1Dlg::OnHide()
{
    this->ShowWindow(SW_HIDE);
}

void CmfcServer1Dlg::OnExit()
{
    sendCloseMsg();
    this->PostMessage(WM_CLOSE);
}

void CmfcServer1Dlg::OnShow()
{
    this->ShowWindow(SW_SHOW);
    SetForegroundWindow();
}

void CmfcServer1Dlg::OnClose()
{
    sendCloseMsg();
    CString str;
    GetDlgItem(IDC_EDIT2)->GetWindowText(str);
    ofstream history("history.txt", ios::out | ios::app);
    history << "\n";
    history.close();
    Shell_NotifyIcon(NIM_DELETE, &nd);
    CDialogEx::OnClose();
}
// 发送关闭服务器的消息
void CmfcServer1Dlg::sendCloseMsg()
{
    if (m_connect) {
        sendMSG(mymsg.join("", TYPE[Server_is_closed], TYPE[AllUser]));
        listenSocket->Close();
        delete listenSocket;
        listenSocket = NULL;
        m_connect = false;
        user_socket.clear();
        SetDlgItemText(IDC_userC, _T("0"));
        GetDlgItem(IDC_PortId)->EnableWindow(1);	//使控件可以修改
        SetDlgItemText(IDC_OpenCloseServer, _T("打开服务器"));
        updateEvent(_T("服务器已关闭."));
    }
}

void CmfcServer1Dlg::OnClearLog()
{
    SetDlgItemText(IDC_EDIT2, _T(""));
}

void CmfcServer1Dlg::OnDropFiles(HDROP hDropInfo)
{
    // 读取文件并发送文件给所有用户
    SetForegroundWindow();		//设置窗口置顶显示
    char filepath[1024] = "";
    DragQueryFile(hDropInfo, 0, filepath, 1024);	// 获取拖放第1个文件的完整文件名
    if (GetFileAttributes(filepath) != FILE_ATTRIBUTE_DIRECTORY) {
        CFile fileSend(filepath, CFile::modeRead);
        long size = (long)fileSend.GetLength();
        fileSend.Close();
        CString s;
        s.Format("暂只支持传输 %dMB 以内的文件！", MAX_PACKAGE_NUM*PACKAGE_SIZE / 1024 / 1024);
        if (size > MAX_PACKAGE_NUM*PACKAGE_SIZE) {
            MessageBox(s, "温馨提示");
            return;
        }
        fileSendName = filepath;
        SetTimer(1, 2, 0);	//设置后台线程读取文件内容
        CString name = filepath;
        name = name.Right(name.GetLength() - name.ReverseFind('\\') - 1);
        static MD5 md5;
        char szMD5[33] = "";
        md5.fileMd5(szMD5, filepath);
        s.Format("%s|%ld|%s", name, size, szMD5);
        sendMSG(mymsg.join(s, TYPE[FileSend], TYPE[AllUser], "服务器"));
    }
    else
        MessageBox("拖入的不是一个有效文件", "温馨提示");

    CDialogEx::OnDropFiles(hDropInfo);
}

void CmfcServer1Dlg::OnTimer(UINT_PTR nIDEvent)
{
    if (0 == nIDEvent) {

    } elif(1 == nIDEvent)
    { //后台读取文件内容并保存
        readFileEnd = 0;
        KillTimer(1);
        memset(packageData, 0, sizeof(packageData));
        CXXFStream fileSend(fileSendName, ios::in | ios::binary);
        long fileSize = (long)fileSend.getSize(), readSize = 0;
        for (unsigned i = 0; readSize < fileSize; i++) {
            if (fileSize - readSize > PACKAGE_SIZE) {		//还有完整的包没有读取
                fileSend.readString(packageData[i], PACKAGE_SIZE);
                readSize += PACKAGE_SIZE;
            }
            else {
                fileSend.readString(packageData[i], fileSize - readSize);
                readSize = fileSize;
            }
        }
        readFileEnd = 1;
        fileSend.close();
        fileSendName = fileSendName.Right(fileSendName.GetLength() - fileSendName.ReverseFind('\\') - 1);
    }
    elif(nIDEvent == 2)
    {	//接收文件监测
        KillTimer(2);
        if (rf.isRecving()) {
            sendMSG(mymsg.join(TYPE[File_Fail], TYPE[AskFileData], fileUser, "服务器"));
            updateEvent("接收文件超时，请稍后再试！", "温馨提示");
            //::MessageBox(GetSafeHwnd(),"接收文件超时，请稍后再试！","温馨提示",0);
            modifyStatus("文件接收超时", 0);
            rf.recvEnd();
        }
    }
    CDialog::OnTimer(nIDEvent);
}

//打开/关闭服务器
void CmfcServer1Dlg::OnOpenCloseServer()
{
    static bool first = 1;
    for (int i = 0; i < userNum; ++i)
        userInfo[i].isOnline = 0;
    getOnlineUserNums();
    if (m_connect) { //如果已连接则关闭服务器
        sendCloseMsg();
        sprintf_s(nd.szTip, "服务器 - 已关闭");	//修改通知栏图标显示文字
        Shell_NotifyIcon(NIM_MODIFY, &nd);
        UpdateData(false);
        return;
    }
    listenSocket = new CServerSocket(this);
    UpdateData(true);
    if (!listenSocket->Create(m_port, SOCK_STREAM)) {	// 创建服务器的套接字
        if (!first)
            AfxMessageBox("创建套接字错误！");
        first = 0;
        listenSocket->Close();
        return;
    }
    //在发送数据的时，不执行由系统缓冲区到socket缓冲区的拷贝，以提高程序的性能
    int nSize = DATA_BUF_SIZE;//设置缓冲区大小
    setsockopt(*listenSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSize, sizeof(int));
    setsockopt(*listenSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nSize, sizeof(int));
    ////禁止Nagle算法（其通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目）
    bool b_noDely = 1;
    setsockopt(*listenSocket, SOL_SOCKET, TCP_NODELAY, (char *)&b_noDely, sizeof(b_noDely));
    if (!listenSocket->Listen(UserNumMax)) {
        CString err;
        err.Format("错误代码：%d", GetLastError());
        if (!first)
            AfxMessageBox("监听失败！" + err);
        first = 0;
        listenSocket->Close();
        return;
    }
    first = 0;
    m_connect = true;
    GetDlgItem(IDC_PortId)->EnableWindow(0);
    UpdateData(false);
    SetDlgItemText(IDC_OpenCloseServer, "关闭服务器");
    updateEvent("服务器已打开.");
    sprintf_s(nd.szTip, "服务器 - 已打开\r\n端口号：%u", m_port);
    Shell_NotifyIcon(NIM_MODIFY, &nd);
}

//用于增加用户，响应用户请求。
void CmfcServer1Dlg::addClient()
{
    CServerSocket* pSocket = new CServerSocket(this);
    listenSocket->Accept(*pSocket);
    pSocket->AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE);
}

//用于移除某个用户,即让其下线
void CmfcServer1Dlg::removeClient(const CString _user)
{
    auto iter = user_socket.find((LPCTSTR)_user);
    if (iter != user_socket.end()) {
        CServerSocket* pSockItem = user_socket[(LPCTSTR)_user];
        user_socket.erase(iter);
        pSockItem->Close();
        delete pSockItem;
    }
}

// 验证用户名和密码是否有效,CheckOnline为1时表示检查在线状态,onlyUser为1时表示只检查该用户是否存在,不验证密码
int CmfcServer1Dlg::isUserInfoValid(bool CheckOnline, bool onlyUser, CString checkUser)
{
    if (checkUser == "")
        checkUser = mymsg.userId;
    for (int i = 0; i < userNum; ++i) {
        if (checkUser == userInfo[i].userName && (onlyUser || mymsg.pw == userInfo[i].pwd)) {	//找到该用户的信息
            if (CheckOnline) {	//检查是否在线
                if (userInfo[i].isOnline)
                    return i;
                else
                    return -1;
            }
            else
                return i;
        }
    }
    return -1;
}

//检查某个用户是否在线
bool CmfcServer1Dlg::isUserOnline(CString _user)
{
    if (_user == "服务器" || _user == "公共聊天室" || _user == TYPE[AllUser])
        return 1;
    for (int i = 0; i < userNum && i < UserNumMax; ++i) {
        if (userInfo[i].userName == _user) {
            if (userInfo[i].isOnline)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

//用于获取数据
void CmfcServer1Dlg::receData(CServerSocket* pSocket)
{
    static char pData[DATA_BUF_SIZE];	//保存接收到的数据
    memset(pData, 0, sizeof(pData));
    static int saveMsg = atoi(AfxGetApp()->GetProfileString("ServerSetting", "saveMsgLog", "-1"));//保存传输日志
    if (saveMsg == -1) {
        AfxGetApp()->WriteProfileString("ServerSetting", "saveMsgLog", "0");
        saveMsg = 0;
    }
    static ofstream save_MSG("ServerMsg.txt");	//保存消息记录的文件
    if (pSocket->Receive(pData, DATA_BUF_SIZE) != SOCKET_ERROR) {
        ///MessageBox(pData, "服务器接收");
        mymsg.load(pData);
        MyMsg msg = mymsg;
        if (saveMsg == 1 && msg.type != TYPE[I_am_online]) //如果不是心跳包消息则保存该条记录
            save_MSG << pData << endl;
        if (msg.userId != "") {
            user_socket[(LPCTSTR)msg.userId] = pSocket; //存储该用户的socket
#pragma region "密码不为空:登录、注销、注册"
            if (msg.pw != "") { //密码不为空
                if (msg.type == TYPE[Login]) {
                    if (msg.userId == "CAPTURE") {
                        sendMSG(msg.join(userList, TYPE[UserList]));
                        updateEvent("_CAPUTURE_ come");
                    }
                    else {
                        int re = isUserInfoValid();	//验证用户身份
                        if (re != -1) {	//用户名和密码有效
                            if (userInfo[re].isOnline) {	//该用户已经在线
                                sendMSG(msg.join("", TYPE[UserIsOnline]));
                                removeClient(msg.userId);
                            }
                            else if (userInfo[re].isRefused) {
                                updateEvent(_T("用户[" + msg.userId + "]请求上线，被拒绝."));
                                return;
                            }
                            else {
                                sendMSG(msg.join(userList, TYPE[UserList]));
                                userInfo[re].isOnline = 1;
                                getOnlineUserNums();
                                UpdateData(false);
                                CString ss;
                                ss.Format("%u", m_userOnlineCount);
                                updateEvent(_T("用户[" + msg.userId + "]上线. 在线用户数：" + ss));
                                string _str;
                                CString Msg; //保存可能存在的离线消息内容
                                ifstream in(OLMSG + "//" + msg.userId);
                                while (!in.eof() && in.is_open()) {
                                    getline(in, _str);
                                    Msg += _str.c_str();
                                }
                                in.close();
                                if (Msg != "") {
                                    Sleep(100);
                                    sendMSG(msg.join(Msg, TYPE[OfflineMsg]));
                                    DeleteFile(OLMSG + "//" + msg.userId);
                                }
                            }
                        }
                        else {
                            sendMSG(msg.join("", TYPE[LoginFail]));
                            removeClient(msg.userId);
                        }
                    }
                }
                elif(msg.type == TYPE[Logout])
                {
                    //用户注销[user]@[pw]###logout
                    if (user_socket.find((LPCTSTR)msg.userId) != user_socket.end()) {
                        CServerSocket* pTemp = user_socket[(LPCTSTR)msg.userId];
                        pTemp->Close();
                        delete pTemp;
                        user_socket.erase(user_socket.find((LPCTSTR)msg.userId));
                        int re = isUserInfoValid();
                        if (re >= 0)
                            userInfo[re].isOnline = 0;	//在线状态改为0
                        getOnlineUserNums();
                        UpdateData(false);
                        updateEvent(_T("用户[" + msg.userId + "]离开."));

                    }
                }
                elif(msg.type == TYPE[Register])
                {
                    //用户注册[user]@[pw]###register
                    if (isUserInfoValid(1, 1) >= 0)	//该用户当前在线，不允许再次注册
                        return;
                    int re = isUserInfoValid(0, 1);
                    if (re >= 0) {					//该用户存在用户列表中但不在线，改写密码
                        strncpy_s(userInfo[re].pwd, msg.pw, 17);
                        updateEvent(_T("用户[" + msg.userId + "]修改密码."));
                    }
                    else {						//该用户不存在，添加到用户列表中
                        strncpy_s(userInfo[userNum].userName, msg.userId, 17);
                        strncpy_s(userInfo[userNum].pwd, msg.pw, 17);
                        userInfo[userNum].isOnline = userInfo[userNum].isRefused = 0;
                        userList += userInfo[userNum].userName;
                        userList += ";";
                        CStdioFile(OLMSG + "//" + CString(userInfo[userNum].userName), CFile::modeCreate | CFile::modeNoTruncate);
                        sendMSG(msg.join(userInfo[userNum].userName, TYPE[AddUserList], TYPE[AllUser]));
                        userNum++;
                        updateEvent(_T("用户[" + msg.userId + "]注册."));
                    }
                    fstream src(DATASRC, ios::out);	//将新用户列表写入数据源
                    for (int i = 0; i < userNum; ++i)
                        src << userInfo[i].userName << "\t" << userInfo[i].pwd << "\n";
                    src.close();
                }
                else {
                    //MessageBox("消息处理失败，包含未知通知——"+msg,"温馨提示");
                    updateEvent("包含未知消息类型:" + msg.type);
                }
            }
#pragma endregion
            else { //没有密码部分
                if (msg.type == TYPE[OnlineState]) {
                    if (msg.data == "服务器" || msg.data == "公共聊天室")
                        sendMSG(msg.join("1", TYPE[OnlineState]));
                    elif(isUserOnline(msg.data))
                        sendMSG(msg.join("1", TYPE[OnlineState]));
                    else
                        sendMSG(msg.join("0", TYPE[OnlineState]));
                }
                elif(msg.type == TYPE[ChatMsg])
                {
                    if (msg.toUser == "公共聊天室") {
                        sendMSG(msg.join(msg.data, TYPE[ChatMsg], TYPE[AllUser], "聊天室-" + msg.userId));
                        updateEvent(msg.data, "用户[" + msg.userId + "]给[聊天室]\t");
                        return;
                    }
                    updateEvent(msg.data, "用户[" + msg.userId + "]给[" + msg.toUser + "]\t");
                    if (isUserOnline(msg.toUser)) { // 目的用户在线则转发数据给目的用户
                        sendMSG(msg.join(msg.data, TYPE[ChatMsg], msg.toUser, msg.userId)); 
                    }
                    else {
                        sendMSG(msg.join("[" + msg.toUser + "]当前不在线，已转为离线消息", TYPE[Status]));// 转为离线消息发送
                        CTime time = CTime::GetCurrentTime();	// 获取系统当前时间
                        ofstream out(OLMSG + "\\" + msg.toUser, ios::out | ios::app);
                        out << msg.join(msg.data, time.Format("%m-%d %H:%M:%S"), "$", msg.userId) << endl;
                        out.close();
                    }
                }
                elif(msg.type == TYPE[I_am_online])
                { //心跳机制发送的验证请求
                  //不做特殊处理
                }
                elif(msg.type == TYPE[FileSend] || msg.type == TYPE[AskFileData] || msg.type == TYPE[FileData])
                {
                    fileTransfer(msg, pData);
                }
            }
            return;
        }
        updateEvent(pData, "unknown\t");
    }
}

//用于更新事件日志，本函数在所有需要更新日志的地方都有调用，方便服务器记录用户的登录和退出事件
void CmfcServer1Dlg::updateEvent(CString str, CString from)
{
    static bool firstEvent = 1;
    CString cstr;
    CTime time = CTime::GetCurrentTime();	// 获取系统当前时间
    str += _T("\r\n");		// 用于换行显示日志
    if (firstEvent) {
        cstr = from + time.Format(_T("%Y/%m/%d %H:%M:%S  ")) + str;	// 格式化当前日期和时间
        firstEvent = 0;
    }
    else
        cstr = from + time.Format(_T("%H:%M:%S  ")) + str;	// 格式化当前时间
    int lastLine = m_event.LineIndex(m_event.GetLineCount() - 1);//获取编辑框最后一行索引
    m_event.SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
    m_event.ReplaceSel(cstr);                     //替换所选那一行的内容
    ofstream history("history.txt", ios::out | ios::app);
    history << cstr.GetBuffer();
    history.close();
    GetDlgItemText(IDC_EDIT2, cstr);
    if (cstr.GetLength() > 10000) {
        cstr = cstr.Right(1000);
        m_event.SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
        m_event.ReplaceSel(cstr);
    }
}

//用于发送消息给某个客户端
void CmfcServer1Dlg::sendMSG(CString str)
{
    MyMsg msg(str);
    if (user_socket.find((LPCTSTR)msg.userId) != user_socket.end()) {
        user_socket[(LPCTSTR)msg.userId]->Send(str, str.GetLength() + 1);
    }
    else if (msg.userId != "服务器") { //如果不是单独给某个用户的,就向所有用户发送
        for (auto & elem : user_socket)
            elem.second->Send(str, str.GetLength() + 1);
        //for (UserSocket::iterator iter = user_socket.begin(); iter != user_socket.end(); ++iter)
        //    iter->second->Send(str, str.GetLength() + 1);
    }
}

//用于处理接收到的指令并控制电脑。
void CmfcServer1Dlg::controlPC(CString ControlMsg)
{
    if (ControlMsg == "open kugou") {            //打开播放器
        ShellExecute(NULL, _T("open"), _T("E:\\Program Files\\KuGou\\KuGou.exe"), NULL, NULL, SW_SHOWNORMAL);
        updateEvent("用户[" + mymsg.userId + "]请求打开酷狗播放器");
    }
    else if (ControlMsg == "open qq") {
        ShellExecute(NULL, _T("open"), "E:\\Program Files\\Tencent\\QQ\\Bin\\QQ.exe", NULL, NULL, SW_SHOWNORMAL);
        updateEvent("用户[" + mymsg.userId + "]请求打开QQ应用");
    }
    else if (ControlMsg == "open chrome") {
        ShellExecute(NULL, _T("open"), "chrome.exe", NULL, NULL, SW_SHOWNORMAL);//C:\\Program Files (x86)\\Google\\Chrome\\Application
        updateEvent("用户[" + mymsg.userId + "]请求打开谷歌浏览器");
    }
    else if (ControlMsg == "close kugou") {       //关闭播放器
        HWND hWnd = ::FindWindow(_T("kugou_ui"), NULL);
        DWORD id_num;
        GetWindowThreadProcessId(hWnd, &id_num);
        HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id_num);
        TerminateProcess(hd, 0);
        updateEvent("用户[" + mymsg.userId + "]请求关闭酷狗播放器");
    }
}

int CmfcServer1Dlg::getOnlineUserNums()
{
    m_userOnlineCount = 0;
    for (int i = 0; i < userNum && i < UserNumMax; ++i) {
        if (userInfo[i].isOnline != 0)
            m_userOnlineCount++;
    }
    return m_userOnlineCount;
}

void CmfcServer1Dlg::fileSend(MyMsg& msg, bool NoAsk/*=0*/)
{
    int i = msg.data.Find('|');
    CString name = msg.data.Left(i);
    CString size = MyMsg::rightN(msg.data, i + 1);
    i = size.Find('|');
    CString fileMD5 = MyMsg::rightN(size, i + 1);
    size = size.Left(i);
    int fileSize = atoi(size);
    if (fileSize > 1024 * 1024)
        size.Format("%.2f MB", fileSize / 1024.0 / 1024.0);
    else if (fileSize > 1024)
        size.Format("%.2f KB", fileSize / 1024.0);
    else
        size.Format("%d 字节", fileSize);
    if (rf.isRecving()) {
        if (!NoAsk) MessageBox('[' + msg.userId + "] 给你发来文件：\n文件名：" + name + "\n文件大小：" + size + "\n当前正在接收文件，请等待当前文件接收完再接收其他文件或终止当前文件接收", "温馨提示", 0);
        return;
    }
    if (NoAsk || MessageBox('[' + msg.userId + "] 给你发来文件：\n文件名：" + name + "\n文件大小：" + size + "\n是否同意接收？",
                            "温馨提示", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        CString fmt = "*" + name.Right(name.GetLength() - name.ReverseFind('.'));
        CFileDialog dlg(false, 0, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, fmt + "|" + fmt + "|All Files(*.*)|*.*||");
        if (NoAsk || dlg.DoModal() == IDOK) {
            fileUser = msg.userId;
            sendMSG(msg.join("0", TYPE[AskFileData], msg.userId, "服务器"));
            CreateDirectory("CaptureFile", 0);
            CString saveFileAt = NoAsk ? ("CaptureFile\\" + name) : dlg.GetPathName();
            rf.init(saveFileAt, fileSize, fileMD5);
            SetTimer(2, 2000, 0);
        }
        else {
            sendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], msg.userId, "服务器"));
        }
    }
    else
        sendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], msg.userId, "服务器"));
}

//关于文件的消息,返回1表示处理成功
int CmfcServer1Dlg::fileTransfer(MyMsg & msg, const char * pData)
{
    static int saveTrans = atoi(AfxGetApp()->GetProfileString("ServerSetting", "saveTransLog", "-1"));//保存传输日志
    if (saveTrans == -1) {
        AfxGetApp()->WriteProfileString("ServerSetting", "saveTransLog", "0");
        saveTrans = 0;
    }
    if (msg.type == TYPE[FileSend]) {
        //发送文件请求，可能是发给服务器的或者是给另一个用户的
        if (msg.toUser == "服务器") {
            SetForegroundWindow();
            fileSend(msg, 1);
        }
        elif(isUserOnline(msg.toUser))
        {
            //检查是否在线。若在线，先存到服务器，再转发给该用户
            sft.set(msg.toUser, msg.userId, 0, msg.data);
            fileUser = msg.userId;
            int i = msg.data.Find('|');
            CString name = msg.data.Left(i);
            CString size = MyMsg::rightN(msg.data, i + 1);
            i = size.Find('|');
            CString fileMD5 = MyMsg::rightN(size, i + 1);
            size = size.Left(i);
            //Sleep(20);
            sendMSG(msg.join("0", TYPE[AskFileData], msg.userId, "服务器"));
            CreateDirectory("ServerFile", 0);
            rf.init("ServerFile\\" + name, atoi(size), fileMD5);
            SetTimer(2, 2000, 0);
            fileSendName = name;
            //sendMSG( msg.join(msg.data,msg.type,msg.toUser,msg.userId) );
        }
    }
    elif(msg.type == TYPE[AskFileData])
    {
        if (msg.toUser != "服务器") {
            /*//策略1：转发给该用户（经常出现问题）
            if(isUserOnline(msg.toUser))
            {
            Sleep(10);
            sendMSG( msg.join(msg.data,msg.type,msg.toUser,msg.userId) );
            }*/
            //策略2：文件数据已先保存到服务器，现在发送给该用户
            if (msg.toUser == sft.fileFromUser) {
                msg.toUser = "服务器";
            }
        }
        if (msg.toUser == "服务器") { //发给服务器
            static fstream dataTrans;
            if (msg.data == TYPE[File_NO]) {		//没有拒绝接收文件
                updateEvent("[" + msg.userId + "]拒绝接收文件：" + fileSendName);
            }
            elif(msg.data == TYPE[File_Over])
            {
                updateEvent("[" + msg.userId + "]已接收文件：" + fileSendName);
                if (saveTrans) dataTrans.close();
            }
            elif(msg.data == TYPE[File_Fail])
            {
                updateEvent("[" + msg.userId + "]未能成功接收文件：" + fileSendName);
                if (saveTrans) dataTrans.close();
            }
            else {			//请求发送数据包
                int dataIndex = atoi(msg.data);
                while (0 == dataIndex && readFileEnd == 0) {
                    Sleep(10);
                    static MSG msg1;
                    if (PeekMessage(&msg1, (HWND)NULL, 0, 0, PM_REMOVE)) {
                        ::SendMessage(msg1.hwnd, msg1.message, msg1.wParam, msg1.lParam);
                    }
                }	//等待读取文件内容结束
                if (dataIndex >= 0 && dataIndex < MAX_PACKAGE_NUM) {
                    if (saveTrans) {
                        if (0 == dataIndex)
                            dataTrans.open("TransLog.txt", ios::out);
                        dataTrans << "发送数据包 " << dataIndex << "\t  " << strlen(packageData[dataIndex]) << endl;
                    }
                    sendMSG(msg.join(packageData[dataIndex], TYPE[FileData], msg.userId, "服务器"));
                }
            }
            if (sft.fileSendOver == 0 && (msg.data == TYPE[File_NO] || msg.data == TYPE[File_Over] || msg.data == TYPE[File_Fail]))
                sendFileToOthers();
        }
    }
    elif(msg.type == TYPE[FileData])
    {
        //文件数据需要接收
        if (msg.toUser == "服务器") {	//服务器保存数据
            if (rf.isRecving()) {
                KillTimer(2);
                rf.addPacketage(strstr(pData, seperator) + strlen(seperator));
                SetTimer(2, 2000, 0);
            }
        }
        else { //转发给目的用户的数据暂存到服务器
         //sendMSG( msg.join(strstr(pData,STR[4])+strlen(STR[4]),msg.type,msg.toUser,msg.userId) );
        }
    }
    else {
        MessageBox(msg.type + ":" + msg.data, "unknown");
        return 0;
    }
    return 1;
}

void CmfcServer1Dlg::sendFileToOthers(bool first)
{
    if (sft.fileSendOver == 0) {
        if (first) { //读取文件内容准备发送
            fileSendName = "ServerFile\\" + fileSendName;
            SetTimer(1, 2, 0);
        }
        static int sendAt = 0;
        if (isUserOnline(sft.fileToUser)) {
            if (sft.fileToUser == "公共聊天室" || sft.fileToUser == TYPE[AllUser]) {
                if (sendAt < userNum) { //还没有发送完毕
                    while (!(userInfo[sendAt].isOnline) || userInfo[sendAt].userName == sft.fileFromUser)
                        ++sendAt;
                    if (sendAt < userNum) {
                        Sleep(20);
                        sendMSG(mymsg.join(sft.fileInfo, TYPE[FileSend], userInfo[sendAt].userName, sft.fileFromUser));
                        ++sendAt;
                    }
                }
            }
            else {
                sendMSG(mymsg.join(sft.fileInfo, TYPE[FileSend], sft.fileToUser, "服务器"));
                sft.fileSendOver = 1;
            }
        }
        if (sendAt >= userNum || !isUserOnline(sft.fileToUser)) {
            sendAt = 0;
            sft.fileSendOver = 1;
        }
    }
}

void CmfcServer1Dlg::modifyStatus(CString sta, bool _sleep)
{
    HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, sta, this->m_hWnd, 0);
    if (_sleep)
        Sleep(10 * _sleep);
    ::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 125, 205));
}

//-----------RecvFile类两个成员函数------------//
//这两个函数使用了theApp,因此不能放在类声明中
void RecvFile::addPacketage(const char *data)
{
    static CmfcServer1Dlg* pDlg = static_cast<CmfcServer1Dlg*>(theApp.GetMainWnd());
    if (packageRecv < packageNum) {
        packageRecv++;
        if (packageRecv < packageNum)		//还有数据包没有接收
            pDlg->sendMSG(pDlg->mymsg.join(getPackRecv(), TYPE[AskFileData], pDlg->fileUser, "服务器"));//请求下一个数据包
        strcpy_s(packageData[packageRecv - 1], data);
    }
    if (pDlg != NULL) {
        static long timeNow = timeStart;
        if (clock() - timeNow > 400) {
            timeNow = clock();
            CString str;
            str.Format("文件已接收 %.1f%%！    用时 %.1fs   平均速度 %.1fk/s", 100.0 * packageRecv / packageNum
                       , (clock() - timeStart) / 1000.0, 1.0*packageRecv / packageNum*fileLength / ((clock() - timeStart)));
            pDlg->modifyStatus(str, 0);
        }
    }
    if (packageNum == packageRecv)
        saveFile(clock() - timeStart);
}

void RecvFile::saveFile(int useTime)
{
    static CmfcServer1Dlg* pDlg = static_cast<CmfcServer1Dlg*>(theApp.GetMainWnd());
    static HWND hWnd = pDlg->GetSafeHwnd();
    bool clear = 1;	//是否清空数据包内容
    if (packageNum == packageRecv) {
        pDlg->modifyStatus("正在校验文件内容！", 0);
        CXXFStream out(fileNewName, ios::out | ios::binary);
        for (int i = 0; i < packageRecv - 1; i++) {
            out.writeString(packageData[i], PACKAGE_SIZE);
        }
        if (fileLength % PACKAGE_SIZE == 0)	//最后一包是完整的
            out.writeString(packageData[packageNum - 1], PACKAGE_SIZE);
        else								//最后一包没有填充满
            out.writeString(packageData[packageNum - 1], fileLength % PACKAGE_SIZE);
        out.close();
        static MD5 md5;
        char newFileMD5[33] = "";
        md5.fileMd5(newFileMD5, (LPCTSTR)fileNewName);
        if (fileMD5 == CString(newFileMD5)) {
            pDlg->sendMSG(pDlg->mymsg.join(TYPE[File_Over], TYPE[AskFileData], pDlg->fileUser, "服务器"));
            timeMsg.Format("    用时 %.1fs   平均速度 %.1fk/s", useTime / 1000.0, 1.0*fileLength / useTime);
            pDlg->modifyStatus("文件已接收完毕！" + timeMsg, 0);
            pDlg->sendFileToOthers(1);
            clear = 0;
            pDlg->updateEvent("接收并保存文件成功", "温馨提示");
            //MessageBox(hWnd,"接收并保存文件成功","温馨提示",0);
        }
        else {
            pDlg->sendMSG(pDlg->mymsg.join(TYPE[File_Fail], TYPE[AskFileData], pDlg->fileUser, "服务器"));
            //MessageBox(hWnd,"接收或保存文件失败，请稍后再试！（错误代码：0x00041)","温馨提示",0);
            pDlg->updateEvent("接收或保存文件失败，请稍后再试！（错误代码：0x00041)", "温馨提示");
            pDlg->modifyStatus("文件接收失败", 0);
            DeleteFile(fileNewName);
        }
    }
    recvEnd(clear);
}
