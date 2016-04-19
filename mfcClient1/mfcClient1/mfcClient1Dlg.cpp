
// mfcClient1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"

#include "CXXFStream.hpp"
#include "md5.h"

using namespace std;

char packageData[MAX_PACKAGE_NUM][2 * PACKAGE_SIZE + 1]; //文件包数据

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//------------------------- CAboutDlg关于对话框类 ---------------------------//
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() : CDialogEx(CAboutDlg::IDD)
    {}

    enum {
        IDD = IDD_ABOUTBOX
    };
};


//---RecvFile类的两个成员函数 :这两个函数使用了theApp,因此不能放在类声明中---//
void RecvFile::addPacketage(const char *data)
{
    static CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    if (packageRecv < packageNum) {
        packageRecv++;
        if (packageRecv < packageNum)		//还有数据包没有接收
            pDlg->sendMSG(pDlg->mymsg.join(getPackRecv(), TYPE[AskFileData], "", "", pDlg->m_fileUser), 0);//请求下一个数据包
        strcpy_s(packageData[packageRecv - 1], data);
        static int showTransLog;//显示传输日志
        GET_WRITE(showTransLog, "showTransLog", "0");
        if (showTransLog) {
            CString str;
            str.Format("已收到%d个数据包,packageNum=%d", packageRecv, packageNum);
            pDlg->updateEvent(str, "系统通知");
        }
    }
    static long timeNow = timeStart;
    if (pDlg != 0) {
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
    static CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    static HWND hWnd = pDlg->GetSafeHwnd();
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
            pDlg->sendMSG(pDlg->mymsg.join(TYPE[File_Over], TYPE[AskFileData], "", "", pDlg->m_fileUser), 0);
            timeMsg.Format("    用时 %.1fs   平均速度 %.1fk/s", useTime / 1000.0, 1.0*fileLength / useTime);
            pDlg->modifyStatus("文件已接收完毕！" + timeMsg, 0);
            pDlg->updateEvent("文件\"" + fileNewName + "\"已接收完毕！" + timeMsg, "系统通知");
            MessageBox(hWnd, "接收并保存文件成功", "温馨提示", 0);
        }
        else {
            pDlg->sendMSG(pDlg->mymsg.join(TYPE[File_Fail], TYPE[AskFileData], "", "", pDlg->m_fileUser), 0);
            pDlg->updateEvent("接收或保存文件\"" + fileNewName + "\"失败，请稍后再试！（错误代码：0x00041)", "系统通知");
            MessageBox(hWnd, "接收或保存文件失败，请稍后再试！（错误代码：0x00041)", "温馨提示", 0);
            pDlg->modifyStatus("文件接收失败", 0);
            DeleteFile(fileNewName);
        }
    }
    recvEnd();
}


//------------------------ ClientInfo类的构造函数 ---------------------------//
int ClientInfo::userNum = 0;
CString ClientInfo::myDIR;

ClientInfo::ClientInfo() {
    pDlg = 0;
    m_readFileEnd = 0;
    m_connected = 0;
    m_DataSend = m_userID = "";

    char path[2048] = "";
    GetTempPath(2048, path);
    strcat(path, "mfcClient1");
    CreateDirectory(path, 0);
    AfxGetApp()->WriteProfileString("ClientSetting", "tempDir", path);
    myDIR = path + CString("\\");
    DeleteFile(myDIR + "send.txt");
    DeleteFile(myDIR + "TransLog.txt");
}


//------------------- CmfcClient1Dlg 类的成员函数实现 -----------------------//

CmfcClient1Dlg::CmfcClient1Dlg(CWnd* pParent)
    : CDialogEx(CmfcClient1Dlg::IDD, pParent)
    , pChatlog(NULL)
    , autoConnect(0)
{
}

CmfcClient1Dlg::~CmfcClient1Dlg()
{
    if (m_connected) {
        sendMSG(mymsg.join("", TYPE[Logout], m_userID, "", "", m_pw), 0);
        pSock->Close();
        delete pSock;
    }
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

BEGIN_MESSAGE_MAP(CmfcClient1Dlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_QUERYDRAGICON()
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_WM_DROPFILES()
    ON_BN_CLICKED(IDC_Connect, &CmfcClient1Dlg::OnConnect)
    ON_BN_CLICKED(IDC_Send, &CmfcClient1Dlg::OnSend)
    ON_BN_CLICKED(IDC_LogOff, &CmfcClient1Dlg::OnLogoff)
    ON_BN_CLICKED(IDC_Chatlog, &CmfcClient1Dlg::OnChatlog)
    ON_CBN_SELCHANGE(IDC_msgTo, &CmfcClient1Dlg::OnCbnSelChangeMsgTo)
END_MESSAGE_MAP()

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

    HWND h = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "就绪!", m_hWnd, 0);
    ::SendMessage(h, SB_SETBKCOLOR, 0, RGB(0, 120, 200));

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
    }
    else {
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
    }
    else {
        CDialogEx::OnPaint();
    }
}

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
            sendMSG(mymsg.join("", TYPE[I_am_online], m_userID, "", ""), 0);
    }
    elif(nIDEvent == 2) {	//接收文件监测
        KillTimer(2);
        if (m_rf.isRecving()) {
            sendMSG(mymsg.join(TYPE[File_Fail], TYPE[AskFileData], m_userID, "", m_fileUser), 0);
            m_rf.recvEnd();
            updateEvent("接收文件" + m_rf.getFileName() + "超时", "系统通知");
            ::MessageBox(GetSafeHwnd(), "接收文件超时，请稍后再试！", "温馨提示", 0);
            modifyStatus("文件接收超时！", 0);
        }
    }
    elif(nIDEvent == 3) {	//拖入文件后自动保存文件内容
        KillTimer(3);
        m_readFileEnd = 0;
        CXXFStream fileStr(m_fileSendName, ios::in | ios::binary);
        long fileSize = (long)fileStr.getSize(), readSize = 0;
        for (unsigned i = 0; readSize < fileSize; i++) {
            memset(packageData[i], 0, 2 * PACKAGE_SIZE + 1);
            if (fileSize - readSize > PACKAGE_SIZE) {		//还有完整的包没有读取
                fileStr.readString(packageData[i], PACKAGE_SIZE);
                readSize += PACKAGE_SIZE;
            }
            else {
                fileStr.readString(packageData[i], fileSize - readSize);
                readSize = fileSize;
            }
        }
        m_readFileEnd = 1;
        fileStr.close();
        m_fileSendName = m_fileSendName.Right(m_fileSendName.GetLength() - m_fileSendName.ReverseFind('\\') - 1);
    }
    elif(nIDEvent == 4) {	//服务器断开后每隔一段时间主动尝试连接
        autoConnect = 1;
        OnConnect();
        autoConnect = 0;
    }
    CDialog::OnTimer(nIDEvent);
}

void CmfcClient1Dlg::OnConnect()
{
    if (m_connected) {    // 如果已经连接，则断开服务器
        m_connected = false;
        sendMSG(mymsg.join("", TYPE[Logout], m_userID, "", "", m_pw), 0);
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
    }
    //未连接，则准备连接服务器
    pSock = new CClientSocket(m_userID);
    if (!pSock->Create()) { //创建套接字失败
        MessageBox(_T("创建套接字失败！"), "温馨提示");
        return;
    }
    //在接收数据的时，不执行由系统缓冲区到socket缓冲区的拷贝，以提高程序的性能
    int nSize = DATA_BUF_SIZE;//设置缓冲区大小
    setsockopt(*pSock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, sizeof(int));
    setsockopt(*pSock, SOL_SOCKET, SO_SNDBUF, (char *)&nSize, sizeof(int));
    //禁止Nagle算法（其通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目）
    bool b_noDely = 1;
    setsockopt(*pSock, SOL_SOCKET, TCP_NODELAY, (char *)&b_noDely, sizeof(b_noDely));

    static bool firstCon = 1; //是否为第一次连接
    if (pSock->Connect(m_ip, m_port)) {  //连接服务器成功
        sendMSG(mymsg.join("", TYPE[Login], m_userID, "", "", m_pw), 0); //连接服务器时发送密码以验证身份
        SetTimer(0, 3000, NULL); //设置登录延时
        m_ConPC.SetWindowText(_T("正在连接服务器..."));
        GetDlgItem(IDC_Connect)->EnableWindow(0); //连接请求送出但还未被回应前屏蔽连接按钮
    }
    else {
        if (!firstCon) {
            if (!autoConnect)
                MessageBox("连接服务器失败：" + pSock->getLastErrorStr(), "温馨提示", MB_ICONERROR);
        }
        else
            firstCon = 0;
        return;
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
        }
        else {
            sendMSG(mymsg.join(m_DataSend, TYPE[ChatMsg], m_userID, "", m_msgTo));
        }
    }
    else {
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
        sendMSG(mymsg.join("", TYPE[Logout], m_userID, "", "", m_pw), 0);
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
    m_toUserIsOnline = 1;
    if (m_msgTo == "服务器" || m_msgTo == "公共聊天室")
        modifyStatus("你可以向[" + m_msgTo + "]自由发送消息");
    else
        sendMSG(mymsg.join(m_msgTo, TYPE[OnlineState], m_userID), 0);
}

void CmfcClient1Dlg::OnDropFiles(HDROP hDropInfo)
{
    SetForegroundWindow();		//设置窗口置顶显示
    if (!m_connected) {
        MBox("连接已断开，请先连接服务器！");
        return;
    }
    if (m_rf.isRecving()) {
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
            if (m_toUserIsOnline != 1) {
                MBox("对方不在线，暂时无法发送离线文件，你可以发送离线消息与他联系");
                break;
            }
            save_SendFileInfo(filepath, size);
        }
        else
            MBox("拖入的不是一个有效文件");
    } while (0);
    DragFinish(hDropInfo);
    CDialogEx::OnDropFiles(hDropInfo);
}

void CmfcClient1Dlg::receData()
{
    KillTimer(1);
    KillTimer(4);
    GetDlgItem(IDC_DataSend)->SetFocus();
    char pData[DATA_BUF_SIZE] = ""; //保存接收到的数据
    pSock->Receive(pData, DATA_BUF_SIZE);
    mymsg.load(pData);
    MyMsg msg = mymsg;
    static const CString myLogFile = myDIR + "ClientLog-" + m_userID + ".txt";
    static int del_one_time = DeleteFile(myLogFile);
    static ofstream logFile(myLogFile, ios::out | ios::app);
    logFile << pData << endl;
    static int showEveryMsg; //保存传输日志
    GET_WRITE(showEveryMsg, "showEveryMsg", "0");
    if (showEveryMsg == 1)
        ::MessageBox(0, pData, "收到消息", 0);
    if (msg.type == TYPE[Server_is_closed]) { //服务器已关闭的消息必定会处理，且首先处理
        sprintf_s(nd.szTip, "客户端：%s - 离线", (LPCTSTR)m_userID);
        Shell_NotifyIcon(NIM_MODIFY, &nd);
        if (m_connected) {    //如果已经连接，则断开服务器
            KillTimer(2);
            if (m_rf.isRecving()) {
                sendMSG(msg.join(TYPE[File_Fail], TYPE[AskFileData], m_fileUser), 0);
                m_rf.recvEnd();
            }
            m_connected = false;
            pSock->Close();
            pSock = NULL;
            m_ConPC.SetWindowText(_T("连接服务器"));
            UpdateData(false);
            GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
            GetDlgItem(IDC_DataSend)->EnableWindow(0);
            GetDlgItem(IDC_Send)->EnableWindow(0);
            GetDlgItem(IDC_msgTo)->EnableWindow(0);
        }
        modifyStatus("Server is closed!");
        SetTimer(4, 2000, 0);
    }
    elif(msg.userId == m_userID) //给自己的消息
    {
        //MessageBox(0,msg.data,"",0);
        if (msg.type == TYPE[ChatMsg]) {
            updateEvent(msg.data, "[" + msg.fromUser + "]:");
            modifyStatus("收到[" + msg.fromUser + "]的新消息！");
        }
        elif(msg.type == TYPE[UserList]) //收到了用户列表，则表示登录成功
        {
            KillTimer(0);
            userNum = 0;
            m_cbMsgTo.ResetContent();
            int i = msg.data.Find(";");
            CString newUser;
            do {
                newUser = msg.data.Left(i);
                if (newUser != m_userID) { //把除自己以外的用户添加到用户列表
                    m_cbMsgTo.AddString(newUser);
                    userNum++;
                }
                msg.data = MyMsg::rightN(msg.data, i + 1);//msg.data.Right(msg.GetLength()-i-1);
                i = msg.data.Find(";");
            } while (i != -1);
            m_cbMsgTo.AddString("服务器");
            m_cbMsgTo.AddString("公共聊天室");
            m_cbMsgTo.SetCurSel(m_cbMsgTo.GetCount() - 1);
            m_cbMsgTo.GetLBText(m_cbMsgTo.GetCurSel(), m_msgTo);	//获取消息传送的目的用户
            if (m_msgTo == "服务器" || m_msgTo == "公共聊天室") {
                modifyStatus("已连接到服务器. 你可以向[" + m_msgTo + "]自由发送消息");
                m_toUserIsOnline = 1;
            }
            else
                sendMSG(msg.join(m_msgTo, TYPE[OnlineState], "", "", "服务器"), 0);
            updateEvent(msg.data, "", 1);
            GetDlgItem(IDC_DataReceive)->EnableWindow(1);
            GetDlgItem(IDC_DataSend)->EnableWindow(1);
            GetDlgItem(IDC_Connect)->EnableWindow(1);
            GetDlgItem(IDC_Send)->EnableWindow(1);
            GetDlgItem(IDC_msgTo)->EnableWindow(1);
            sprintf_s(nd.szTip, "客户端：%s - 在线", (LPCTSTR)m_userID);	//状态在登陆判重时用到
            Shell_NotifyIcon(NIM_MODIFY, &nd);
            m_connected = true;
            m_ConPC.SetWindowText(_T("断开服务器"));
            UpdateData(false);
        }
        elif(msg.type == TYPE[OnlineState])
        {
            CString sel;
            GetDlgItemText(IDC_msgTo, sel);
            if (msg.data == "1") {
                modifyStatus("[" + sel + "]当前在线");
                m_toUserIsOnline = 1;
            }
            else if (msg.data == "0") {
                modifyStatus("[" + sel + "]当前不在线");
                m_toUserIsOnline = 0;
            }
        }
        elif(msg.type == TYPE[FileSend])  //这里可以接受服务器发给自己的文件请求
        {
            SetForegroundWindow();
            fileSend(msg);
        }
        elif(msg.type == TYPE[FileData])
        {
            if (m_rf.isRecving()) {
                KillTimer(2);
                m_rf.addPacketage(msg.data);
                SetTimer(2, m_fileTimeOut, 0);
            }
        }
        elif(msg.type == TYPE[AskFileData])
        {
            static fstream dataTrans;
            static int saveTrans = atoi(AfxGetApp()->GetProfileString("ClientSetting", "saveTransLog", "-1"));//保存传输日志
            if (saveTrans == -1) {
                AfxGetApp()->WriteProfileString("ClientSetting", "saveTransLog", "0");
                saveTrans = 0;
            }
            if (msg.data == TYPE[File_NO])	//拒绝接收该文件
                updateEvent("[" + msg.fromUser + "]拒绝接收文件：“" + m_fileSendName + "”", "[" + msg.fromUser + "]");
            else if (msg.data == TYPE[File_Over]) {
                updateEvent("[" + msg.fromUser + "]已接收文件：“" + m_fileSendName + "”", "[" + msg.fromUser + "]");
                if (saveTrans) dataTrans.close();
            }
            else if (msg.data == TYPE[File_Fail]) {
                updateEvent("[" + msg.fromUser + "]未能成功接收文件：“" + m_fileSendName + "”", "[" + msg.fromUser + "]");
                if (saveTrans) dataTrans.close();
            }
            else {			//请求发送数据包
                int dataIndex = atoi(msg.data);
                while (0 == dataIndex && m_readFileEnd == 0) {
                    Sleep(10);
                    static MSG msg1;
                    if (PeekMessage(&msg1, (HWND)NULL, 0, 0, PM_REMOVE)) {
                        ::SendMessage(msg1.hwnd, msg1.message, msg1.wParam, msg1.lParam);
                    }
                }	//等待读取文件内容结束
                if (dataIndex >= 0 && dataIndex < MAX_PACKAGE_NUM) {
                    if (0 == dataIndex)
                        m_fileUser = msg.fromUser;
                    if (saveTrans) {
                        if (0 == dataIndex)
                            dataTrans.open(myDIR + "TransLog.txt", ios::out);
                        dataTrans << "发送数据包 " << dataIndex << "\t  " << strlen(packageData[dataIndex]) << endl;
                    }
                    sendMSG(msg.join(packageData[dataIndex], TYPE[FileData], "", "", m_fileUser), 0);
                    static int showTransLog;//显示传输日志
                    GET_WRITE(showTransLog, "showTransLog", "0");
                    if (showTransLog) {
                        CString str;
                        str.Format("已发送数据包%d(%d),packageNum=%d", dataIndex, strlen(packageData[dataIndex]), m_rf.getPackNum());
                        updateEvent(str, "系统通知");
                    }
                    //MessageBox(packageData[dataIndex],"tmpMsg");
                }
            }
        }
        elif(msg.type == TYPE[LoginFail]) //用户验证失败
        {
            KillTimer(0);
            MessageBox("用户名或密码错误，请检查！\r\n如果没有账号请注册新账号！", "登陆失败");
            ShowWindow(SW_HIDE);
            login();
            ShowWindow(SW_SHOW);
        }
        elif(msg.type == TYPE[UserIsOnline]) //用户已经在线
        {
            KillTimer(0);
            MessageBox("该用户已经在线,请勿重复登陆！", "登陆失败");
            ShowWindow(SW_HIDE);
            login();
            ShowWindow(SW_SHOW);
        }
        elif(msg.type == TYPE[OfflineMsg]) //离线消息
        {
            static MyMsg olmsg;
            CString tmpMsg(msg.data);
            do {
                tmpMsg = olmsg.load(tmpMsg, 1);
                updateEvent(olmsg.type + "  " + olmsg.data, "[" + olmsg.fromUser + "]:", 0, 3);
            } while (tmpMsg.Find(seperator) != -1);
            updateEvent("—————————————以上是离线消息—————————————", "", 0, 3);
            modifyStatus("收到离线消息！");
        }
        elif(msg.type == TYPE[Status]) //修改状态
        {
            modifyStatus(msg.data);
        }
    }
    elif(msg.userId == TYPE[AllUser]) //发给所有用户的
    {
        if (msg.type == TYPE[ChatMsg]) {
            if (msg.fromUser != ("聊天室-" + m_userID)) { //过滤自己收到来自自己的消息
                updateEvent(msg.data, "[" + msg.fromUser + "]:");
                modifyStatus("收到[" + msg.fromUser + "]的新消息！");
            }
        }
        elif(msg.type == TYPE[AddUserList])
        {
            m_cbMsgTo.InsertString(m_cbMsgTo.GetCount() - 2, msg.data);
            userNum++;
        }
        elif(msg.type == TYPE[FileSend]) //这里可以接受服务器群发的文件请求
        {
            SetForegroundWindow();
            fileSend(msg);
        }
    }
    SetTimer(1, 2000, 0);	//心跳
}

//用于发送消息给服务器
void CmfcClient1Dlg::sendMSG(const CString &send, bool upEvent)
{
    static MyMsg tmp_msg;
    tmp_msg.load(send);
    //MessageBox(0,send,"客户端发送",0);
    if (pSock->Send(send, send.GetLength() + 1) == SOCKET_ERROR) {	//发送消息失败
        CString err = pSock->getLastErrorStr();
        if (tmp_msg.type != TYPE[I_am_online]) {
            MessageBox("发送失败：" + err, "温馨提示", 0);
        }
        else {
            KillTimer(1);
            sprintf_s(nd.szTip, "客户端：%s - 离线", (LPCTSTR)m_userID);
            Shell_NotifyIcon(NIM_MODIFY, &nd);
            if (m_connected) {    // 如果已经连接，则断开服务器
                m_connected = false;
                pSock->Close();
                pSock = NULL;
                m_ConPC.SetWindowText(_T("连接服务器"));
                UpdateData(false);
                GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
                GetDlgItem(IDC_DataSend)->EnableWindow(0);
                GetDlgItem(IDC_Send)->EnableWindow(0);
                GetDlgItem(IDC_msgTo)->EnableWindow(0);
            }
            SetTimer(4, 2000, 0);
            MessageBox("与服务器连接异常，请尝试重连: " + err, "温馨提示", 0);
        }
        return;
    }
    if (tmp_msg.type != TYPE[I_am_online]) {
        static ofstream logFile(myDIR + "send.txt", ios::out | ios::app);
        logFile << "send:" << send << endl;
    }
    if (upEvent && tmp_msg.pw == "") {	//没有密码部分
        if (tmp_msg.type == TYPE[ChatMsg]) {
            static CString lastUser = "", nowUser = "";
            nowUser = tmp_msg.toUser;
            CString data = tmp_msg.data;
            if (nowUser != lastUser) {
                lastUser = nowUser;
                tmp_msg.data = "——————————与[" + nowUser + "]聊天——————————";
                updateEvent(tmp_msg.data, "", 0, 3);
            }
            tmp_msg.data = data;
            updateEvent(tmp_msg.data, "我:");
        }
        CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
        m_DataSend = "";
        SetDlgItemText(IDC_DataSend, "");	// 清空发送窗口的数据
    }
}

void CmfcClient1Dlg::updateEvent(const CString &showMsg, const CString &from,
                                 bool reset, int timeFMT)
{
    CString str, sysMsg;
    CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    GetDlgItemText(IDC_DataReceive, str);
    CTime time = CTime::GetCurrentTime();	// 获取系统当前时间
    if (str == "" && timeFMT == 2) //消息为空则显示日期和时间
        timeFMT = 1;
    switch (timeFMT) {
        case 1:
            sysMsg = from + "  " + time.Format(_T("%Y/%m/%d %H:%M:%S  "));
            break;
        case 2:
            sysMsg = from + "  " + time.Format(_T("%H:%M:%S  "));
            break;
        case 3:
            sysMsg = from == "" ? from : from + "  ";
        default:
            break;
    }
    if (str != "" && reset) {
        str = "—————————————以上是历史消息—————————————\r\n";
    }
    else {
        if (showMsg != "")
            str = sysMsg + showMsg + "\r\n";
    }
    CEdit* m_event = (CEdit*)GetDlgItem(IDC_DataReceive);
    int lastLine = m_event->LineIndex(m_event->GetLineCount() - 1);//获取编辑框最后一行索引
    m_event->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
    m_event->ReplaceSel(str);                     //替换所选那一行的内容
    ofstream myDIR_userID(myDIR + m_userID, ios::out | ios::app);
    myDIR_userID << str;
    myDIR_userID.close();
    GetDlgItemText(IDC_DataReceive, str);
    if (str.GetLength() > 15000) {
        str = str.Right(1000);
        SetDlgItemText(IDC_DataReceive, str);
        m_event->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
        m_event->ReplaceSel("");                     //替换所选那一行的内容
    }
}

void CmfcClient1Dlg::fileSend(MyMsg& msg)
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
    if (m_rf.isRecving()) {
        CString tips = '[' + msg.fromUser + "] 给你发来文件：\n文件名：" + name + "\n文件大小："
            + size + "\n当前正在接收文件，请等待当前文件接收完再接收其他文件或终止当前文件接收";
        MessageBox(tips, "温馨提示");
        return;
    }
    GET_WRITE(m_fileTimeOut, "fileTimeOut", "2000");
    CString tips = '[' + msg.fromUser + "] 给你发来文件：\n文件名：" + name + "\n文件大小：" + size
        + "\n是否同意接收？";
    if (MessageBox(tips, "温馨提示", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        CString fmt = "*" + name.Right(name.GetLength() - name.ReverseFind('.'));
        CFileDialog dlg(false, 0, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, fmt + "|" + fmt + "|All Files(*.*)|*.*||");
        if (dlg.DoModal() == IDOK) {
            m_fileUser = msg.fromUser;
            sendMSG(msg.join("0", TYPE[AskFileData], "", "", m_fileUser), 0);
            m_rf.init(dlg.GetPathName(), fileSize, fileMD5);
            SetTimer(2, m_fileTimeOut, 0);
            modifyStatus("准备接收文件！");
        }
        else
            sendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], "", "", msg.fromUser), 0);
    }
    else
        sendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], "", "", msg.fromUser), 0);
}

void CmfcClient1Dlg::modifyStatus(const CString &sta, bool _sleep)	//修改状态栏
{
    HWND h = CreateStatusWindow(WS_CHILD | WS_VISIBLE, sta, m_hWnd, 0);
    if (_sleep)
        Sleep(20);
    ::SendMessage(h, SB_SETBKCOLOR, 0, RGB(0, 125, 205));
}

bool CmfcClient1Dlg::login()
{
    CLoginDlg loginDlg;
    if (loginDlg.DoModal() != IDOK)  //弹出登录对话框
        return false;
    m_userID = loginDlg.userID;
    m_pw = loginDlg.pw;
    m_ip = loginDlg.ip;
    m_port = loginDlg.port;
    SetWindowText("客户端 - " + m_userID);
    OnConnect();
    return true;
}

void CmfcClient1Dlg::save_SendFileInfo(const CString &filepath, long size)
{
    if (m_rf.isRecving()) {
        MBox("当前正在接收文件，为了不影响数据接收，请等待当前传输完成后再发送文件！");
        return;
    }
    m_fileSendName = filepath;
    CString name = m_fileSendName, s;
    SetTimer(3, 1, 0);	//设置后台线程读取文件内容
    name = name.Right(name.GetLength() - name.ReverseFind('\\') - 1);
    static MD5 md5;
    char szMD5[33] = "";
    md5.fileMd5(szMD5, filepath);
    s.Format("%s|%d|%s", name, size, szMD5);
    sendMSG(mymsg.join(s, TYPE[FileSend], m_userID, "", m_msgTo), 0);
    m_rf.setPackNum((size + PACKAGE_SIZE - 1) / PACKAGE_SIZE);
}
