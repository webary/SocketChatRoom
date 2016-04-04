
// mfcClient1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"

CString myDIR;			//个人文件夹，位于临时目录
extern CString TYPE[30];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // 对话框数据
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CmfcClient1Dlg 对话框


CmfcClient1Dlg::~CmfcClient1Dlg()
{
    delete pChatlog;
    Shell_NotifyIcon(NIM_DELETE, &nd);
}

void CmfcClient1Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_Connect, m_ConPC);
    DDX_Control(pDX, IDC_msgTo, m_cbMsgTo);
    DDX_Text(pDX, IDC_DataSend, m_DataSend);
}

#pragma region MessageMap
BEGIN_MESSAGE_MAP(CmfcClient1Dlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_Connect, &CmfcClient1Dlg::OnConnect)
    ON_BN_CLICKED(IDC_Send, &CmfcClient1Dlg::OnSend)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_LogOff, &CmfcClient1Dlg::OnLogoff)
    ON_BN_CLICKED(IDC_Chatlog, &CmfcClient1Dlg::OnChatlog)
    ON_CBN_SELCHANGE(IDC_msgTo, &CmfcClient1Dlg::OnCbnSelChangeMsgTo)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()
#pragma endregion

BOOL CmfcClient1Dlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        CWnd *pw = CWnd::GetFocus();
        if (pw == GetDlgItem(IDC_DataSend)) {
            OnSend();
        }
        return true;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// CmfcClient1Dlg 消息处理程序

BOOL CmfcClient1Dlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。
    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标
    //TODO:在此处添加自定义代码
    LONG style = ::GetWindowLong(GetSafeHwnd(), GWL_STYLE);
    style &= ~WS_THICKFRAME;//使窗口不能用鼠标改变大小
    ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, style);

    HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, "就绪!", this->m_hWnd, 0);
    ::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 120, 200));

    GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
    GetDlgItem(IDC_DataSend)->EnableWindow(0);
    GetDlgItem(IDC_Connect)->EnableWindow(0);
    GetDlgItem(IDC_Send)->EnableWindow(0);

    nd.cbSize = sizeof(NOTIFYICONDATA);
    nd.hWnd = m_hWnd;
    nd.uID = IDR_MAINFRAME;
    nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nd.uCallbackMessage = 0;
    nd.hIcon = m_hIcon;
    sprintf_s(nd.szTip, "客户端 - 登陆");
    Shell_NotifyIcon(NIM_ADD, &nd);
    pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    if (!login()) //如果点了取消或者关闭则直接退出主对话框
        OnCancel();
    return 0;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmfcClient1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CmfcClient1Dlg::OnPaint()
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
    } else {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CmfcClient1Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CmfcClient1Dlg::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == 0) {	//登录监测
        if (KillTimer(0)) {
            pSock->Close();
            SetForegroundWindow();
            MessageBox("连接请求超时，请重试！", "温馨提示", MB_ICONERROR);
            ShowWindow(SW_HIDE);
            login();
            ShowWindow(SW_SHOW);
        }
    }
    elif(nIDEvent == 1) {	//心跳机制
        if (m_connected)
            pSock->SendMSG(pSock->mymsg.join("", TYPE[I_am_online], pSock->userID, "", ""), 0);
    }
    elif(nIDEvent == 2) {	//接收文件监测
        KillTimer(2);
        if (rf.isRecving()) {
            pSock->SendMSG(pSock->mymsg.join(TYPE[File_Fail], TYPE[AskFileData], pSock->userID, "", fileUser), 0);
            rf.recvEnd();
            pSock->updateEvent("接收文件" + rf.getFileName() + "超时", "系统通知");
            ::MessageBox(GetSafeHwnd(), "接收文件超时，请稍后再试！", "温馨提示", 0);
            modifyStatus("文件接收超时！", 0);
        }
    }
    elif(nIDEvent == 3) {	//拖入文件后自动保存文件内容
        KillTimer(3);
        readFileEnd = 0;
        CXXFStream fileStr(fileSendName, std::ios::in | std::ios::binary);
        long fileSize = (long)fileStr.getSize(), readSize = 0;
        for (unsigned i = 0; readSize < fileSize; i++) {
            memset(packageData[i], 0, 2 * PACKAGE_SIZE + 1);
            if (fileSize - readSize > PACKAGE_SIZE) {		//还有完整的包没有读取
                fileStr.readString(packageData[i], PACKAGE_SIZE);
                readSize += PACKAGE_SIZE;
            } else {
                fileStr.readString(packageData[i], fileSize - readSize);
                readSize = fileSize;
            }
        }
        readFileEnd = 1;
        fileStr.close();
        fileSendName = fileSendName.Right(fileSendName.GetLength() - fileSendName.ReverseFind('\\') - 1);
    }
    elif(nIDEvent == 4) {	//服务器断开后每隔一段时间主动尝试连接
        OnConnect();
    }
    CDialog::OnTimer(nIDEvent);
}

void CmfcClient1Dlg::OnConnect()
{
    if (m_connected) {    // 如果已经连接，则断开服务器
        m_connected = false;
        pSock->SendMSG(pSock->mymsg.join("", TYPE[Logout], pSock->userID, "", "", m_pw), 0);
        pSock->Close();
        pSock = NULL;
        m_ConPC.SetWindowText(_T("连接服务器"));
        UpdateData(false);
        GetDlgItem(IDC_Connect)->EnableWindow(1);
        GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
        GetDlgItem(IDC_DataSend)->EnableWindow(0);
        GetDlgItem(IDC_Send)->EnableWindow(0);
        GetDlgItem(IDC_msgTo)->EnableWindow(0);
        sprintf_s(nd.szTip, "客户端：%s - 离线", (LPCTSTR)m_userID);
        Shell_NotifyIcon(NIM_MODIFY, &nd);
        modifyStatus("已断开服务器");
        KillTimer(1);
        return;
    } else { // 未连接，则连接服务器
        pSock = new CClientSocket(m_userID);
        if (!pSock->Create()) {       //创建套接字
            MessageBox(_T("创建套接字失败！"), "温馨提示");
            return;
        }
    }
    //在接收数据的时，不执行由系统缓冲区到socket缓冲区的拷贝，以提高程序的性能
    int nSize = DATA_BUF_SIZE;//设置缓冲区大小
    setsockopt(*pSock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, sizeof(int));
    setsockopt(*pSock, SOL_SOCKET, SO_SNDBUF, (char *)&nSize, sizeof(int));
    /*//禁止Nagle算法（其通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目）
    bool b_noDely = 1;
    setsockopt( *pSock, SOL_SOCKET, TCP_NODELAY, ( char * )&b_noDely, sizeof( b_noDely ) );*/
    if (!pSock->Connect(m_ip, m_port)) {  //连接服务器失败
        if (!firstCon)
            MessageBox("连接服务器失败：" + getLastErrorStr(), "温馨提示");
        return;
    } else {
        pSock->SendMSG(pSock->mymsg.join("", TYPE[Login], pSock->userID, "", "", m_pw), 0); //连接服务器时发送密码以验证身份
        SetTimer(0, 3000, NULL); //设置登录延时
        m_ConPC.SetWindowText(_T("正在连接服务器..."));
        GetDlgItem(IDC_Connect)->EnableWindow(0); //连接请求送出但还未被回应前屏蔽连接按钮
    }
    firstCon = 0;
    GetDlgItem(IDC_DataSend)->SetFocus();
}

void CmfcClient1Dlg::OnSend()
{
    if (!m_connected) {
        MessageBox("请先连接服务器", "温馨提示");
        return;                               //未连接服务器则不执行
    }
    UpdateData(true);                         //获取控件数据
    if (m_DataSend != "") {
        if (m_msgTo == m_userID) {
            MessageBox("请不要给自己发送消息", "温馨提示");
        } else {
            pSock->SendMSG(pSock->mymsg.join(m_DataSend, TYPE[ChatMsg], pSock->userID, "", m_msgTo));
        }
    } else {
        MessageBox("请先输入内容", "温馨提示");
    }
    GetDlgItem(IDC_DataSend)->SetFocus();
}

void CmfcClient1Dlg::OnOK()
{
    OnSend();
}

void CmfcClient1Dlg::OnLogoff()
{
    KillTimer(4);
    if (m_connected) {    // 如果已经连接，则断开服务器
        m_connected = false;
        pSock->SendMSG(pSock->mymsg.join("", TYPE[Logout], pSock->userID, "", "", m_pw), 0);
        pSock->Close();
        pSock = NULL;
        GetDlgItem(IDC_DataSend)->SetWindowText("");
    }
    sprintf_s(nd.szTip, "客户端 - 登陆");
    Shell_NotifyIcon(NIM_MODIFY, &nd);
    GetDlgItem(IDC_DataReceive)->SetWindowText("");
    ShowWindow(SW_HIDE);
    if (!login())
        OnCancel();
    ShowWindow(SW_SHOW);
}

void CmfcClient1Dlg::OnChatlog()
{
    delete pChatlog;
    pChatlog = new CChatLogDlg(m_userID);
    pChatlog->Create(IDD_ChatLog);//创建一个非模态对话框
    pChatlog->ShowWindow(SW_SHOWNORMAL); //显示非模态对话框
    int lastLine = pChatlog->p_editCL->LineIndex(pChatlog->p_editCL->GetLineCount() - 1);
    pChatlog->p_editCL->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
    pChatlog->p_editCL->ReplaceSel(pChatlog->chatLog);   //替换所选那一行的内容
}

void CmfcClient1Dlg::OnCbnSelChangeMsgTo()
{
    m_cbMsgTo.GetLBText(m_cbMsgTo.GetCurSel(), m_msgTo);	//获取消息传送的目的用户
    to_isOnline = 1;
    if (m_msgTo == "服务器" || m_msgTo == "公共聊天室")
        modifyStatus("你可以向[" + m_msgTo + "]自由发送消息");
    else
        pSock->SendMSG(pSock->mymsg.join(m_msgTo, TYPE[OnlineState], pSock->userID), 0);
}

void CmfcClient1Dlg::OnDropFiles(HDROP hDropInfo)
{
    SetForegroundWindow();		//设置窗口置顶显示
    if (!m_connected) {
        MBox("连接已断开，请先连接服务器！");
        return;
    }
    if (rf.isRecving()) {
        MBox("当前正在接收文件，为了不影响数据接收，请等待当前传输完成后再发送文件！");
        return;
    }
    // 读取文件并发送文件给服务器或转发给某个用户
    int  nFileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 256);   //获取拖入的文件数量
    char filepath[1024] = "";
    DragQueryFile(hDropInfo, 0, filepath, 1024);	// 获取拖放第1个文件的完整文件名
    do {
        if (GetFileAttributes(filepath) != FILE_ATTRIBUTE_DIRECTORY) {
            CFile fileSend(filepath, CFile::modeRead);
            long size = (long)fileSend.GetLength();
            fileSend.Close();
            if (size <= 0) {
                MBox("文件为空，请检查！");
                break;
            }
            elif(size > MAX_PACKAGE_NUM*PACKAGE_SIZE) {
                CString s;
                s.Format("暂只支持传输 %dMB 以内的文件！", MAX_PACKAGE_NUM*PACKAGE_SIZE / 1024 / 1024);
                MBox(s);
                break;
            }
            if (to_isOnline != 1) {
                MBox("对方不在线，暂时无法发送离线文件，你可以发送离线消息与他联系");
                break;
            }
            save_SendFileInfo(filepath, size);
        } else
            MBox("拖入的不是一个有效文件");
    } while (0);
    DragFinish(hDropInfo);
    CDialogEx::OnDropFiles(hDropInfo);
}



#include "md5.h"
bool ClientInfo::login()
{
    logining = 1;
    CLoginDlg loginDlg;		//连接的主对话框
    // 检验用户名和密码是否有效
    if (loginDlg.DoModal() != IDOK)
        return 0;
    m_userID = loginDlg.userID;
    m_pw = loginDlg.pw;
    m_ip = loginDlg.ip;
    m_port = loginDlg.port;
    pDlg->SetWindowText("客户端 - " + m_userID);
    pDlg->OnConnect();
    logining = 0;
    return 1;
}

void ClientInfo::modifyStatus(CString sta, bool _sleep)	//修改状态栏
{
    HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, sta, pDlg->m_hWnd, 0);
    if (_sleep)
        Sleep(10 * _sleep);
    ::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 125, 205));
}

void ClientInfo::save_SendFileInfo(const char* filepath, long size)
{
    if (rf.isRecving()) {
        pDlg->MBox("当前正在接收文件，为了不影响数据接收，请等待当前传输完成后再发送文件！");
        return;
    }
    fileSendName = filepath;
    CString name = fileSendName, s;
    pDlg->SetTimer(3, 1, 0);	//设置后台线程读取文件内容
    name = name.Right(name.GetLength() - name.ReverseFind('\\') - 1);
    static MD5 md5;
    char szMD5[33] = "";
    md5.fileMd5(szMD5, filepath);
    s.Format("%s|%d|%s", name, size, szMD5);
    pSock->SendMSG(pSock->mymsg.join(s, TYPE[FileSend], pSock->userID, "", m_msgTo), 0);
    rf.setPackNum((size + PACKAGE_SIZE - 1) / PACKAGE_SIZE);
}
