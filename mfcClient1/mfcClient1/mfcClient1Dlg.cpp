
// mfcClient1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"
#include "afxdialogex.h"
#include "md5.hpp"

#define IP_LAN		 "172.27.35.3"
#define IP_LOCALHOST "127.0.0.1"
#define IP_SERVER	 "120.25.207.230"

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
{
}

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
    ON_BN_CLICKED(IDC_Connect, &CmfcClient1Dlg::OnBnClickedConnect)
    ON_BN_CLICKED(IDC_Send, &CmfcClient1Dlg::OnBnClickedSend)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_LogOff, &CmfcClient1Dlg::OnBnClickedLogoff)
    ON_BN_CLICKED(IDC_Chatlog, &CmfcClient1Dlg::OnBnClickedChatlog)
    ON_CBN_SELCHANGE(IDC_msgTo, &CmfcClient1Dlg::OnCbnSelChangeMsgTo)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()
#pragma endregion

BOOL CmfcClient1Dlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN) {
        CWnd *pw = CWnd::GetFocus();
        if (pw == GetDlgItem (IDC_DataSend)) {
            OnBnClickedSend();
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

    LONG style = ::GetWindowLong(this->GetSafeHwnd(), GWL_STYLE);
    style &= ~WS_THICKFRAME;//使窗口不能用鼠标改变大小
    ::SetWindowLong(this->GetSafeHwnd(), GWL_STYLE, style);

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
    if(!loginDlg()) {
        PostQuitMessage(0);
    }
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
    if (nIDEvent==0) {		//登录监测
        if(KillTimer(0)) {
            pSock->Close();
            SetForegroundWindow();
            ::MessageBox(GetSafeHwnd(),"连接请求超时，请重试！","温馨提示",0);
            ShowWindow(SW_HIDE);
            loginDlg();
            ShowWindow(SW_SHOW);
        }
    }
    elif(nIDEvent==1) {		//心跳机制
        if(m_connect)
            pSock->SendMSG(pSock->mymsg.join("",TYPE[I_am_online],"","",""),0);
    }
    elif(nIDEvent==2) {		//接收文件监测
        KillTimer(2);
        if(rf.isRecving()) {
            pSock->SendMSG(pSock->mymsg.join(TYPE[File_Fail],TYPE[AskFileData],"","",fileUser),0);
            rf.recvEnd();
            pSock->updateEvent("接收文件"+rf.getFileName()+"超时","系统通知");
            ::MessageBox(GetSafeHwnd(),"接收文件超时，请稍后再试！","温馨提示",0);
            modifyStatus("文件接收超时！",0);
        }
    }
    elif(nIDEvent==3) {		//拖入文件后自动保存文件内容
        KillTimer(3);
        readFileEnd = 0;
        CXXFStream fileStr(fileSendName,ios::in | ios::binary);
        long fileSize = (long)fileStr.getSize(),readSize = 0;
        for(unsigned i=0; readSize<fileSize; i++) {
            memset(packageData[i],0,2*PACKAGE_SIZE+1);
            if(fileSize-readSize>PACKAGE_SIZE) {		//还有完整的包没有读取
                fileStr.readString(packageData[i],PACKAGE_SIZE);
                readSize += PACKAGE_SIZE;
            } else {
                fileStr.readString(packageData[i],fileSize-readSize);
                readSize = fileSize;
            }
        }
        readFileEnd = 1;
        fileStr.close();
        fileSendName = fileSendName.Right(fileSendName.GetLength()-fileSendName.ReverseFind('\\')-1);
    }
    elif(nIDEvent==4) {		//服务器断开后每隔一段时间主动尝试连接
        OnBnClickedConnect();
    }
    CDialog::OnTimer(nIDEvent);
}

void CmfcClient1Dlg::OnBnClickedConnect()
{
    if (m_connect) {    // 如果已经连接，则断开服务器
        m_connect = false;
        pSock->SendMSG( pSock->mymsg.join("",TYPE[Logout],"","","",m_pw) ,0);
        pSock->Close();
        pSock = NULL;
        m_ConPC.SetWindowText(_T("连接服务器"));
        UpdateData(false);
        GetDlgItem(IDC_Connect)->EnableWindow(1);
        GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
        GetDlgItem(IDC_DataSend)->EnableWindow(0);
        GetDlgItem(IDC_Send)->EnableWindow(0);
        GetDlgItem(IDC_msgTo)->EnableWindow(0);
        sprintf_s(nd.szTip, "客户端：%s - 离线",m_userID);
        Shell_NotifyIcon(NIM_MODIFY, &nd);
        modifyStatus("已断开服务器");
        KillTimer(1);
        return;
    } else {                                            // 未连接，则连接服务器
        pSock = new CClientSocket(m_userID);
        if (!pSock->Create()) {       //创建套接字
            MessageBox(_T("创建套接字失败！"),"温馨提示");
            return;
        }
    }
    //在接收数据的时，不执行由系统缓冲区到socket缓冲区的拷贝，以提高程序的性能
    int nSize = DATA_BUF_SIZE;//设置缓冲区大小
    setsockopt( *pSock, SOL_SOCKET, SO_RCVBUF, ( char * )&nSize, sizeof( int ) );
    setsockopt( *pSock, SOL_SOCKET, SO_SNDBUF, ( char * )&nSize, sizeof( int ) );
    ////禁止Nagle算法（其通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目）
    /*bool b_noDely = 1;
    setsockopt( *pSock, SOL_SOCKET, TCP_NODELAY, ( char * )&b_noDely, sizeof( b_noDely ) );*/
    if (!pSock->Connect(_T(m_ip), m_port)) {  //连接服务器失败
        if (!firstCon) {
            CString str;
            str.Format("（错误代码：%d）", pSock->getError());
            MessageBox("连接服务器失败！" + str, "温馨提示");
        }
        return;
    } else {
        pSock->SendMSG( pSock->mymsg.join("",TYPE[Login],"","","",m_pw),0 );	//连接服务器时发送密码以验证身份
        SetTimer(0,3000,NULL);		//设置延时
        m_ConPC.SetWindowText(_T("正在连接服务器..."));
        GetDlgItem(IDC_Connect)->EnableWindow(0);	//连接请求送出但还未被回应前屏蔽连接按钮
    }
    firstCon = 0;
    GetDlgItem(IDC_DataSend)->SetFocus();
}

void CmfcClient1Dlg::OnBnClickedSend()
{
    if (!m_connect) {
        MessageBox("请先连接服务器", "温馨提示");
        return;                               //未连接服务器则不执行
    }
    UpdateData(true);                         //获取控件数据
    if (m_DataSend != "") {
        if(m_msgTo==m_userID) {
            MessageBox("请不要给自己发送消息", "温馨提示");
        } else {
            pSock->SendMSG( pSock->mymsg.join(m_DataSend,TYPE[ChatMsg],"","",m_msgTo));
        }
    } else {
        MessageBox("请先输入内容", "温馨提示");
    }
    GetDlgItem(IDC_DataSend)->SetFocus();
}

void CmfcClient1Dlg::OnOK()
{
    OnBnClickedSend();
}

void CmfcClient1Dlg::OnBnClickedLogoff()
{
    KillTimer(4);
    if (m_connect) {    // 如果已经连接，则断开服务器
        m_connect = false;
        pSock->SendMSG( pSock->mymsg.join("",TYPE[Logout],"","","",m_pw) ,0);
        pSock->Close();
        pSock = NULL;
        GetDlgItem(IDC_DataSend)->SetWindowText("");
    }
    sprintf_s(nd.szTip, "客户端 - 登陆");
    Shell_NotifyIcon(NIM_MODIFY, &nd);
    GetDlgItem(IDC_DataReceive)->SetWindowText("");
    ShowWindow(SW_HIDE);
    if(!loginDlg())
        PostQuitMessage(0);
    ShowWindow(SW_SHOW);
}

void CmfcClient1Dlg::OnBnClickedChatlog()
{
    delete pChatlog;
    pChatlog = new CChatLogDlg(m_userID);
    pChatlog->Create(IDD_ChatLog);//创建一个非模态对话框
    pChatlog->ShowWindow(SW_SHOWNORMAL); //显示非模态对话框
    int lastLine =  pChatlog->p_editCL->LineIndex( pChatlog->p_editCL->GetLineCount() - 1);
    pChatlog->p_editCL->SetSel(lastLine+1,lastLine+2, 0);	//选择编辑框最后一行
    pChatlog->p_editCL->ReplaceSel(pChatlog->chatLog);   //替换所选那一行的内容
}

void CmfcClient1Dlg::OnCbnSelChangeMsgTo()
{
    m_cbMsgTo.GetLBText(m_cbMsgTo.GetCurSel(),m_msgTo);	//获取消息传送的目的用户
    to_isOnline = 1;
    if(m_msgTo=="服务器" || m_msgTo=="公共聊天室")
        modifyStatus("你可以向["+m_msgTo+"]自由发送消息");
    else
        pSock->SendMSG( pSock->mymsg.join(m_msgTo,TYPE[OnlineState]) ,0);
}

void CmfcClient1Dlg::OnDropFiles(HDROP hDropInfo)
{
    SetForegroundWindow();		//设置窗口置顶显示
    if(!m_connect) {
        MBox("连接已断开，请先连接服务器！");
        return;
    }
    if(rf.isRecving()) {
        MBox("当前正在接收文件，为了不影响数据接收，请等待当前传输完成后再发送文件！");
        return;
    }
    // 读取文件并发送文件给服务器或转发给某个用户
    int  nFileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 256);   //获取拖入的文件数量
    char filepath[1024]="";
    DragQueryFile(hDropInfo,0,filepath,1024);	// 获取拖放第1个文件的完整文件名
    do {
        if(GetFileAttributes(filepath)!=FILE_ATTRIBUTE_DIRECTORY) {
            CFile fileSend(filepath,CFile::modeRead);
            long size = (long)fileSend.GetLength();
            fileSend.Close();
            if(size<=0) {
                MBox("文件为空，请检查！");
                break;
            }
            elif(size>MAX_PACKAGE_NUM*PACKAGE_SIZE) {
                CString s;
                s.Format("暂只支持传输 %dMB 以内的文件！",MAX_PACKAGE_NUM*PACKAGE_SIZE/1024/1024);
                MBox(s);
                break;
            }
            if(to_isOnline!=1) {
                MBox("对方不在线，暂时无法发送离线文件，你可以发送离线消息与他联系");
                break;
            }
            save_SendFileInfo(filepath,size);
        } else
            MBox("拖入的不是一个有效文件");
    } while(0);
    DragFinish(hDropInfo);
    CDialogEx::OnDropFiles(hDropInfo);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDSET, &CLoginDlg::OnBnClickedSet)
    ON_BN_CLICKED(IDC_regist, &CLoginDlg::OnBnClickedregist)
    ON_BN_CLICKED(IDC_CHECK_remPw, &CLoginDlg::OnBnClickedCheckrempw)
    ON_BN_CLICKED(IDC_Swap, &CLoginDlg::OnBnClickedSwap)
    ON_BN_CLICKED(IDC_ShowPw, &CLoginDlg::OnBnClickedShowpw)
END_MESSAGE_MAP()


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_userID, userID);
    DDV_MaxChars(pDX, userID, 16);
    DDX_Text(pDX, IDC_pw, pw);
    DDV_MaxChars(pDX, pw, 16);
    DDX_Control(pDX, IDC_IPADDRESS1, m_IPAddr);
    DDX_Text(pDX, IDC_EDIT1, port);
}

bool CLoginDlg::onlyAlNum(const CString s)
{
    for(int i=s.GetLength()-1; i>=0; --i)
        if(s[i]>0 && !isalnum(s[i]))	//不是字母不是数字，也不是中文字符
            return 0;
    return 1;
}
//检查数据是否存在无效信息
bool CLoginDlg::DataInvalide()
{
    UpdateData(1);
    m_IPAddr.GetWindowText(ip);
    userID.Replace(" ", "");
    pw.Replace(" ", "");
    if (userID == "") {
        MessageBox("用户名不能为空", "温馨提示");
        GetDlgItem(IDC_userID)->SetFocus();
        return 1;
    }
    if(!onlyAlNum(userID)) {
        MessageBox("用户名只能包含字母、数字或汉字", "温馨提示");
        CEdit* edit = (CEdit*)GetDlgItem(IDC_userID);
        edit->SetFocus();
        edit->SetSel(0,20);
        return 1;
    }
    if (pw == "") {
        MessageBox("密码不能为空", "温馨提示");
        GetDlgItem(IDC_pw)->SetFocus();
        return 1;
    }
    if (port<1024 || port>65535) {
        MessageBox("端口号设置有误，有效范围为1024-65535！", "温馨提示");
        GetDlgItem(IDC_EDIT1)->SetFocus();
        return 1;
    }
    OnBnClickedCheckrempw();
    return 0;
}

void CLoginDlg::OnBnClickedOk()
{
    if(DataInvalide()==1)
        return;
    CClientSocket sock(userID+"*test");
    if (!sock.Create()) {       //创建套接字
        MessageBox(_T("创建套接字失败！"), "温馨提示");
        return;
    }
    if (!sock.Connect(_T(ip), port)) {  //连接服务器
        CString str;
        str.Format("（错误代码：%d）", sock.getError());
        MessageBox("连接服务器失败！" + str, "温馨提示");
        return;
    }
    sock.Close();
    AfxGetApp()->WriteProfileString("Login","IP",ip);
    CDialogEx::OnOK();
}

void CLoginDlg::OnBnClickedSet()
{
    const int dt = 2;
    if (b_strech) {
        for (int i = 0; m_rc.Height() - i >= 200; i += dt) {
            SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height() - i, SWP_NOMOVE | SWP_SHOWWINDOW);
            Sleep(1);
        }
        b_strech = FALSE;
    } else {
        for (int i = 0; i + 200 <= m_rc.Height(); i += dt) {
            SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), i + 200, SWP_NOMOVE | SWP_SHOWWINDOW);
            Sleep(1);
        }
        b_strech = TRUE;
    }
}

BOOL CLoginDlg::OnInitDialog()
{
    ((CButton*)GetDlgItem(IDC_CHECK_remPw))->SetCheck(TRUE);
    CDialog::OnInitDialog();
    CString ipAddr = AfxGetApp()->GetProfileString("Login","IP",IP_SERVER);	//需要连接的服务器ip地址
    m_IPAddr.SetWindowText(ipAddr);
    b_strech = FALSE;
    CString id_pw = AfxGetApp()->GetProfileString("UserInfo", "userID_pw");
    int i = id_pw.Find(STR[0]);
    if(i!=-1) {
        userID = id_pw.Left(i);
        pw = id_pw.Right(id_pw.GetLength()-i-3);
    } else {
        char _user[17]="", _pw[17]="";
        ifstream user_pw(myDIR+"user_pw.dat",ios::in);
        if(!user_pw.fail()) {
            user_pw>>_user>>_pw;
            user_pw.close();
            if(_user[0]!=0 && _pw[0]!=0) {
                userID = _user;
                pw = _pw;
            }
        } else {
            MessageBox("很感谢亲的试用哦，首次使用请先注册一个自己的账号吧！","温馨提示");
        }
    }
    UpdateData(0);
    GetWindowRect(&m_rc);
    SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), 200, SWP_NOMOVE | SWP_SHOWWINDOW);
    GetDlgItem(IDC_userID)->SetFocus();
    return TRUE;
}

void CLoginDlg::OnBnClickedregist()
{
    if(DataInvalide())
        return;
    SetTimer(0,3000,NULL);		//设置延时
    CClientSocket sock(userID);
    if (!sock.Create()) {       //创建套接字
        MessageBox(_T("注册失败——创建套接字失败！"), "温馨提示");
        return;
    }
    if (!sock.Connect(_T(ip), port)) {  //连接服务器
        CString str;
        str.Format("（错误代码：%d）", sock.getError());
        MessageBox("注册失败——连接服务器失败！" + str, "温馨提示");
        puts("\a");
        return;
    }
    if(KillTimer(0)) {
        sock.SendMSG( sock.mymsg.join("",TYPE[Register],"","","",pw),0);
        sock.Close();
        MessageBox("注册信息发送成功，服务器审核通过后即可登陆！", "温馨提示");
        OnOK();
    }
}

void CLoginDlg::OnBnClickedCheckrempw()
{
    if(userID!="" && pw!="") {
        if(BST_CHECKED==((CButton*)GetDlgItem(IDC_CHECK_remPw))->GetCheck()) {
            ofstream user_pw(myDIR+"user_pw.dat");
            user_pw<<userID<<"\t"<<pw;
            user_pw.close();
            AfxGetApp()->WriteProfileString("UserInfo", "userID_pw", userID+ STR[0] +pw);
        } else {
            DeleteFile(myDIR+"user_pw.dat");
            AfxGetApp()->WriteProfileString("UserInfo", "userID_pw", "");
        }
    }
}

void CLoginDlg::OnBnClickedSwap()
{
    if(BST_CHECKED==((CButton*)GetDlgItem(IDC_Swap))->GetCheck()) {
        m_IPAddr.SetWindowText(IP_LOCALHOST);
    } else {
        m_IPAddr.SetWindowText(IP_SERVER);
    }
}

void CLoginDlg::OnBnClickedShowpw()
{
    CEdit *edit = (CEdit*)GetDlgItem(IDC_pw);
    if(BST_CHECKED==((CButton*)GetDlgItem(IDC_ShowPw))->GetCheck()) {
        edit->SetPasswordChar(0);
    } else {
        edit->SetPasswordChar(_T('*'));
    }
    edit->RedrawWindow(NULL,NULL);
}

BEGIN_MESSAGE_MAP(CChatLogDlg, CDialogEx)
    ON_BN_CLICKED(IDC_DelAllLog, &CChatLogDlg::OnBnClickedDelalllog)
    ON_BN_CLICKED(IDC_Update, &CChatLogDlg::OnBnClickedUpdate)
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
    char c_str[1024];
    ifstream tmpF(myDIR + userID,ios::in);
    while(!tmpF.eof() && tmpF.is_open()) {
        getline(tmpF,_str);
        if(_str!="") {
            strncpy_s(c_str,&_str[0],1024);
            chatLog += c_str;
            chatLog += "\r\n";
        }
    }
    tmpF.close();
    if(chatLog=="")
        chatLog = "暂时无任何聊天记录";
    return TRUE;
}

void CChatLogDlg::OnBnClickedDelalllog()
{
    if(chatLog != "暂时无任何聊天记录"
       && MessageBox("***确认删除所有聊天记录吗？此操作不可撤回！***","删除确认",MB_YESNO)==IDYES) {
        DeleteFile(myDIR+userID);
        MessageBox("已删除所有聊天记录！","温馨提示");
    } else if(chatLog == "暂时无任何聊天记录") {
        MessageBox("暂时无任何聊天记录","温馨提示");
    } else {
        return;
    }
    ::SendMessage(GetSafeHwnd(),WM_CLOSE,0,0);
}

void CChatLogDlg::OnBnClickedUpdate()
{
    chatLog = "";
    string _str;
    char c_str[1024];
    ifstream tmpF(myDIR + userID,ios::in);
    while(!tmpF.eof() && tmpF.is_open()) {
        getline(tmpF,_str);
        if(_str!="") {
            strncpy_s(c_str,&_str[0],1024);
            chatLog += c_str;
            chatLog += "\r\n";
        }
    }
    tmpF.close();
    if(chatLog=="")
        chatLog = "暂时无任何聊天记录";
    SetDlgItemText(IDC_Chatlog,"");
    int lastLine =  p_editCL->LineIndex( p_editCL->GetLineCount() - 1);
    p_editCL->SetSel(lastLine+1,lastLine+2, 0);	//选择编辑框最后一行
    p_editCL->ReplaceSel(chatLog);   //替换所选那一行的内容
}


int  CClientDlg::loginDlg()
{
    logining = 1;
    bool loginOK = 0;
    // 检验用户名和密码是否有效
    if (login.DoModal() != IDOK)
        return 0;
    m_userID = login.userID;
    m_pw = login.pw;
    m_ip = login.ip;
    m_port = login.port;
    pDlg->SetWindowText("客户端 - " + m_userID);
    pDlg->OnBnClickedConnect();
    logining = 0;
    return 1;
}

void CClientDlg::modifyStatus(CString sta,bool _sleep)	//修改状态栏
{
    HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, sta, pDlg->m_hWnd, 0);
    if(_sleep)
        Sleep(10 * _sleep);
    ::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 125, 205));
}

void CClientDlg::save_SendFileInfo(const char* filepath,long size)
{
    if(rf.isRecving()) {
        pDlg->MBox("当前正在接收文件，为了不影响数据接收，请等待当前传输完成后再发送文件！");
        return;
    }
    fileSendName = filepath;
    CString name = fileSendName ,s;
    pDlg->SetTimer(3,1,0);	//设置后台线程读取文件内容
    name = name.Right(name.GetLength()-name.ReverseFind('\\')-1);
    static MD5 md5;
    char szMD5[33] = "";
    md5.fileMd5(szMD5,filepath);
    s.Format("%s|%d|%s",name,size,szMD5);
    pSock->SendMSG( pSock->mymsg.join(s,TYPE[FileSend],"","",m_msgTo),0);
    rf.setPackNum((size+PACKAGE_SIZE-1)/PACKAGE_SIZE);
}
