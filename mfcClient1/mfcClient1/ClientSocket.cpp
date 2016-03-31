#include "stdafx.h"
#include "ClientSocket.h"
#include "mfcClient1.h"
#include "mfcClient1Dlg.h"
#include "md5.h"

using namespace std;

CString CClientSocket::userID = "";

char packageData[MAX_PACKAGE_NUM][2 * PACKAGE_SIZE + 1];

CString TYPE[30] = { TYPE_ChatMsg , TYPE_Server_is_closed , TYPE_UserList , TYPE_OnlineState , TYPE_FileSend , TYPE_FileData , TYPE_AskFileData , TYPE_File_NO , TYPE_File_Over , TYPE_File_Fail , TYPE_LoginFail , TYPE_UserIsOnline , TYPE_OfflineMsg , TYPE_AllUser , TYPE_AddUserList , TYPE_I_am_online , TYPE_Logout , TYPE_Login , TYPE_Register , TYPE_Status };
CString STR[5] = { "@@@","<<<",">>>","&&&","###" };//用户名+密码+来自+去向+类型+内容

#define GETS(y,z) AfxGetApp()->GetProfileString("ClientSetting",y,z)
#define WRITE(x,y,z) { char *s = x;if(x==0) s="ClientSetting"; AfxGetApp()->WriteProfileString(s,y,z);}
//x保存的整型变量，y键码，z键值的默认值
#define GET_WRITE(x,y,z) {\
	if((x=atoi(GETS(y,"-1")))==-1){\
		x = atoi(z);\
		WRITE(0,y,z);\
	}\
}
//这两个函数使用了theApp,因此不能放在类声明中
void RecvFile::addPacketage(const char *data)
{
    static CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    if (packageRecv < packageNum) {
        packageRecv++;
        if (packageRecv < packageNum)		//还有数据包没有接收
            pDlg->pSock->SendMSG(pDlg->pSock->mymsg.join(getPackRecv(), TYPE[AskFileData], "", "", pDlg->fileUser), 0);//请求下一个数据包
        strcpy_s(packageData[packageRecv - 1], data);
        static int showTransLog;//显示传输日志
        GET_WRITE(showTransLog, "showTransLog", "0");
        if (showTransLog) {
            CString str;
            str.Format("已收到%d个数据包,packageNum=%d", packageRecv, packageNum);
            pDlg->pSock->updateEvent(str, "系统通知");
        }
    }
    static long timeNow = timeStart;
    if (pDlg != 0) {
        if (clock() - timeNow>400) {
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
            pDlg->pSock->SendMSG(pDlg->pSock->mymsg.join(TYPE[File_Over], TYPE[AskFileData], "", "", pDlg->fileUser), 0);
            timeMsg.Format("    用时 %.1fs   平均速度 %.1fk/s", useTime / 1000.0, 1.0*fileLength / useTime);
            pDlg->modifyStatus("文件已接收完毕！" + timeMsg, 0);
            pDlg->pSock->updateEvent("文件\"" + fileNewName + "\"已接收完毕！" + timeMsg, "系统通知");
            MessageBox(hWnd, "接收并保存文件成功", "温馨提示", 0);
        } else {
            pDlg->pSock->SendMSG(pDlg->pSock->mymsg.join(TYPE[File_Fail], TYPE[AskFileData], "", "", pDlg->fileUser), 0);
            pDlg->pSock->updateEvent("接收或保存文件\"" + fileNewName + "\"失败，请稍后再试！（错误代码：0x00041)", "系统通知");
            MessageBox(hWnd, "接收或保存文件失败，请稍后再试！（错误代码：0x00041)", "温馨提示", 0);
            pDlg->modifyStatus("文件接收失败", 0);
            DeleteFile(fileNewName);
        }
    }
    recvEnd();
}

CClientSocket::CClientSocket(CString _user)
{
    pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    hWnd = pDlg->GetSafeHwnd();
    userID = _user;
}

CClientSocket::~CClientSocket()
{}

void CClientSocket::updateEvent(CString showMsg, CString from, bool reset, int timeFMT)
{
    CString str, sysMsg;
    CmfcClient1Dlg* pDlg = (CmfcClient1Dlg*)theApp.GetMainWnd();
    pDlg->GetDlgItemText(IDC_DataReceive, str);
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
    } else {
        if (showMsg != "")
            str = sysMsg + showMsg + "\r\n";
    }
    CEdit* m_event = (CEdit*)pDlg->GetDlgItem(IDC_DataReceive);
    int lastLine = m_event->LineIndex(m_event->GetLineCount() - 1);//获取编辑框最后一行索引
    m_event->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
    m_event->ReplaceSel(str);                     //替换所选那一行的内容
    ofstream myDIR_userID(myDIR + userID, ios::out | ios::app);
    myDIR_userID << str;
    myDIR_userID.close();
    pDlg->GetDlgItemText(IDC_DataReceive, str);
    if (str.GetLength() > 15000) {
        str = str.Right(1000);
        pDlg->SetDlgItemText(IDC_DataReceive, str);
        m_event->SetSel(lastLine + 1, lastLine + 2, 0);	//选择编辑框最后一行
        m_event->ReplaceSel("");                     //替换所选那一行的内容
    }
}

//	说明：当客户端接收到服务器端发的数据时会响应接收函数OnReceive
void CClientSocket::OnReceive(int nErrorCode)
{
    pDlg->KillTimer(1);
    pDlg->KillTimer(4);
    pDlg->GetDlgItem(IDC_DataSend)->SetFocus();
    char pData[DATA_BUF_SIZE] = ""; //保存接收到的数据
    Receive(pData, DATA_BUF_SIZE);
    mymsg.load(pData);
    MyMsg msg = mymsg;
    static const CString myLogFile = myDIR + "ClientLog-" + userID + ".txt";
    static int del_one_time = DeleteFile(myLogFile);
    static ofstream logFile(myLogFile, ios::out | ios::app);
    logFile << pData << endl;
    static int showEveryMsg;//保存传输日志
    GET_WRITE(showEveryMsg, "showEveryMsg", "0");
    if (showEveryMsg == 1)
        MessageBox(0, pData, "收到消息", 0);
    if (msg.type == TYPE[Server_is_closed]) {
        sprintf_s(pDlg->nd.szTip, "客户端：%s - 离线", (LPCTSTR)userID);
        Shell_NotifyIcon(NIM_MODIFY, &pDlg->nd);
        if (pDlg->m_connect) {    // 如果已经连接，则断开服务器
            pDlg->KillTimer(2);
            if (pDlg->rf.isRecving()) {
                pDlg->pSock->SendMSG(msg.join(TYPE[File_Fail], TYPE[AskFileData], pDlg->fileUser), 0);
                pDlg->rf.recvEnd();
            }
            pDlg->m_connect = false;
            pDlg->pSock->Close();
            pDlg->pSock = NULL;
            pDlg->m_ConPC.SetWindowText(_T("连接服务器"));
            pDlg->UpdateData(false);
            pDlg->GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
            pDlg->GetDlgItem(IDC_DataSend)->EnableWindow(0);
            pDlg->GetDlgItem(IDC_Send)->EnableWindow(0);
            pDlg->GetDlgItem(IDC_msgTo)->EnableWindow(0);
        }
        pDlg->modifyStatus("Server is closed!");
        pDlg->firstCon = 1;
        pDlg->SetTimer(4, 2000, 0);
    }
    elif(msg.userId == userID) { //给自己的消息
        if (pDlg->logining) {	//正在登陆则把前一次的连接断开
            pDlg->pSock->SendMSG(msg.join("", TYPE[Logout], "", "", "", pDlg->m_pw), 0);
        } else {
            //MessageBox(0,msg.data,"",0);
            if (msg.type == TYPE[ChatMsg]) {
                updateEvent(msg.data, "[" + msg.fromUser + "]:");
                pDlg->modifyStatus("收到[" + msg.fromUser + "]的新消息！");
            }
            elif(msg.type == TYPE[UserList]) {	//收到了用户列表，则表示登录成功
                pDlg->userNum = 0;
                pDlg->m_cbMsgTo.ResetContent();
                int i = msg.data.Find(";");
                CString newUser;
                do {
                    newUser = msg.data.Left(i);
                    if (newUser != userID) {	//把除自己以外的用户添加到用户列表
                        pDlg->m_cbMsgTo.AddString(newUser);
                        pDlg->userNum++;
                    }
                    msg.data = rightN(msg.data, i + 1);//msg.data.Right(msg.GetLength()-i-1);
                    i = msg.data.Find(";");
                } while (i != -1);
                pDlg->m_cbMsgTo.AddString("服务器");
                pDlg->m_cbMsgTo.AddString("公共聊天室");
                pDlg->m_cbMsgTo.SetCurSel(pDlg->m_cbMsgTo.GetCount() - 1);
                pDlg->m_cbMsgTo.GetLBText(pDlg->m_cbMsgTo.GetCurSel(), pDlg->m_msgTo);	//获取消息传送的目的用户
                if (pDlg->m_msgTo == "服务器" || pDlg->m_msgTo == "公共聊天室") {
                    pDlg->modifyStatus("已连接到服务器. 你可以向[" + pDlg->m_msgTo + "]自由发送消息");
                    pDlg->to_isOnline = 1;
                } else
                    SendMSG(msg.join(pDlg->m_msgTo, TYPE[OnlineState], "", "", "服务器"), 0);
                updateEvent(msg.data, "", 1);
                pDlg->KillTimer(0);
                pDlg->GetDlgItem(IDC_DataReceive)->EnableWindow(1);
                pDlg->GetDlgItem(IDC_DataSend)->EnableWindow(1);
                pDlg->GetDlgItem(IDC_Connect)->EnableWindow(1);
                pDlg->GetDlgItem(IDC_Send)->EnableWindow(1);
                pDlg->GetDlgItem(IDC_msgTo)->EnableWindow(1);
                sprintf_s(pDlg->nd.szTip, "客户端：%s - 在线", (LPCTSTR)userID);	//状态在登陆判重时用到
                Shell_NotifyIcon(NIM_MODIFY, &pDlg->nd);
                pDlg->m_connect = true;
                pDlg->m_ConPC.SetWindowText(_T("断开服务器"));
                pDlg->UpdateData(false);
            }
            elif(msg.type == TYPE[OnlineState]) {
                CString sel;
                pDlg->GetDlgItemText(IDC_msgTo, sel);
                if (msg.data == "1") {
                    pDlg->modifyStatus("[" + sel + "]当前在线");
                    pDlg->to_isOnline = 1;
                } else if (msg.data == "0") {
                    pDlg->modifyStatus("[" + sel + "]当前不在线");
                    pDlg->to_isOnline = 0;
                }
            }
            elif(msg.type == TYPE[FileSend]) {
                //这里可以接受服务器发给自己的文件请求
                pDlg->SetForegroundWindow();
                fileSend(msg);
            }
            elif(msg.type == TYPE[FileData]) {
                if (pDlg->rf.isRecving()) {
                    pDlg->KillTimer(2);
                    pDlg->rf.addPacketage(msg.data);
                    pDlg->SetTimer(2, pDlg->fileTimeOut, 0);
                }
            }
            elif(msg.type == TYPE[AskFileData]) {
                static fstream dataTrans;
                static int saveTrans = atoi(AfxGetApp()->GetProfileString("ClientSetting", "saveTransLog", "-1"));//保存传输日志
                if (saveTrans == -1) {
                    AfxGetApp()->WriteProfileString("ClientSetting", "saveTransLog", "0");
                    saveTrans = 0;
                }
                if (msg.data == TYPE[File_NO])	//拒绝接收该文件
                    updateEvent("[" + msg.fromUser + "]拒绝接收文件：“" + pDlg->fileSendName + "”", "[" + msg.fromUser + "]");
                else if (msg.data == TYPE[File_Over]) {
                    updateEvent("[" + msg.fromUser + "]已接收文件：“" + pDlg->fileSendName + "”", "[" + msg.fromUser + "]");
                    if (saveTrans) dataTrans.close();
                } else if (msg.data == TYPE[File_Fail]) {
                    updateEvent("[" + msg.fromUser + "]未能成功接收文件：“" + pDlg->fileSendName + "”", "[" + msg.fromUser + "]");
                    if (saveTrans) dataTrans.close();
                } else {			//请求发送数据包
                    int dataIndex = atoi(msg.data);
                    while (0 == dataIndex && pDlg->readFileEnd == 0) {
                        Sleep(10);
                        static MSG msg1;
                        if (PeekMessage(&msg1, (HWND)NULL, 0, 0, PM_REMOVE)) {
                            ::SendMessage(msg1.hwnd, msg1.message, msg1.wParam, msg1.lParam);
                        }
                    }	//等待读取文件内容结束
                    if (dataIndex >= 0 && dataIndex < MAX_PACKAGE_NUM) {
                        if (0 == dataIndex)
                            pDlg->fileUser = msg.fromUser;
                        if (saveTrans) {
                            if (0 == dataIndex)
                                dataTrans.open(myDIR + "TransLog.txt", ios::out);
                            dataTrans << "发送数据包 " << dataIndex << "\t  " << strlen(packageData[dataIndex]) << endl;
                        }
                        SendMSG(msg.join(packageData[dataIndex], TYPE[FileData], "", "", pDlg->fileUser), 0);
                        static int showTransLog;//显示传输日志
                        GET_WRITE(showTransLog, "showTransLog", "0");
                        if (showTransLog) {
                            CString str;
                            str.Format("已发送数据包%d(%d),packageNum=%d", dataIndex, strlen(packageData[dataIndex]), pDlg->rf.getPackNum());
                            pDlg->pSock->updateEvent(str, "系统通知");
                        }
                        //MessageBox(packageData[dataIndex],"tmpMsg");
                    }
                }
            }
            elif(msg.type == TYPE[LoginFail]) {
                //用户验证失败
                if (!pDlg->logining) {	//没有在登录才进行这些操作，正在登录则不接收该消息
                    pDlg->KillTimer(0);
                    MessageBox(hWnd, "用户名或密码错误，请检查！\r\n如果没有账号请注册新账号！", "登陆失败", 0);
                    pDlg->ShowWindow(SW_HIDE);
                    pDlg->login();
                    pDlg->ShowWindow(SW_SHOW);
                }
            }
            elif(msg.type == TYPE[UserIsOnline]) {
                //用户已经在线
                if (0==strstr(pDlg->nd.szTip, "在线")) {//拒绝正在登陆的用户
                    pDlg->KillTimer(0);
                    MessageBox(hWnd, "该用户已经在线,请勿重复登陆！", "登陆失败", 0);
                    pDlg->ShowWindow(SW_HIDE);
                    pDlg->login();
                    pDlg->ShowWindow(SW_SHOW);
                } else
                    MessageBox(hWnd, "你的账号在另一处登陆，如果不是你本人操作，请及时修改密码！", "安全提示", 0);
            }
            elif(msg.type == TYPE[OfflineMsg]) {	//离线消息
                static MyMsg olmsg;
                CString tmpMsg(msg.data);
                do {
                    tmpMsg = olmsg.load(tmpMsg, 1);
                    updateEvent(olmsg.type + "  " + olmsg.data, "[" + olmsg.fromUser + "]:", 0, 3);
                } while (tmpMsg.Find(STR[0]) != -1);
                updateEvent("—————————————以上是离线消息—————————————", "", 0, 3);
                pDlg->modifyStatus("收到离线消息！");
            }
            elif(msg.type == TYPE[Status]) { //修改状态
                pDlg->modifyStatus(msg.data);
            }
        }
    }
    elif(msg.userId == TYPE[AllUser]) {	//发给所有用户的
        if (msg.type == TYPE[ChatMsg]) {
            if (msg.fromUser != ("聊天室-" + userID)) { //过滤自己收到来自自己的消息
                updateEvent(msg.data, "[" + msg.fromUser + "]:");
                pDlg->modifyStatus("收到[" + msg.fromUser + "]的新消息！");
            }
        }
        elif(msg.type == TYPE[AddUserList]) {
            pDlg->m_cbMsgTo.InsertString(pDlg->m_cbMsgTo.GetCount() - 2, msg.data);
            pDlg->userNum++;
        }
        elif(msg.type == TYPE[FileSend]) { //这里可以接受服务器群发的文件请求
            pDlg->SetForegroundWindow();
            fileSend(msg);
        }
    }
    pDlg->SetTimer(1, 2000, 0);	//心跳
    CSocket::OnReceive(nErrorCode);
}

/*SendMSG函数用于向服务器发送消息，函数会在主对话框类中调用。*/
void CClientSocket::SendMSG(CString send, bool upEvent)
{
    static MyMsg tmp_msg;
    tmp_msg.load(send);
    //MessageBox(0,send,"客户端发送",0);
    if (Send(send, send.GetLength() + 1) == SOCKET_ERROR) {	//发送消息失败
        CString err = getLastErrorStr();
        if (tmp_msg.type != TYPE[I_am_online]) {
            MessageBox(hWnd, "发送失败：" + err, "温馨提示", 0);
        } else {
            pDlg->KillTimer(1);
            sprintf_s(pDlg->nd.szTip, "客户端：%s - 离线", (LPCTSTR)userID);
            Shell_NotifyIcon(NIM_MODIFY, &pDlg->nd);
            if (pDlg->m_connect) {    // 如果已经连接，则断开服务器
                pDlg->m_connect = false;
                pDlg->pSock->Close();
                pDlg->pSock = NULL;
                pDlg->m_ConPC.SetWindowText(_T("连接服务器"));
                pDlg->UpdateData(false);
                pDlg->GetDlgItem(IDC_DataReceive)->EnableWindow(0);	//禁用这些控件
                pDlg->GetDlgItem(IDC_DataSend)->EnableWindow(0);
                pDlg->GetDlgItem(IDC_Send)->EnableWindow(0);
                pDlg->GetDlgItem(IDC_msgTo)->EnableWindow(0);
            }
            pDlg->firstCon = 1;
            pDlg->SetTimer(4, 2000, 0);
            MessageBox(hWnd, "与服务器连接异常，请尝试重连: " + err, "温馨提示", 0);
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
        pDlg->m_DataSend = "";
        pDlg->SetDlgItemText(IDC_DataSend, "");	// 清空发送窗口的数据
    }
}

void CClientSocket::fileSend(MyMsg& msg)
{
    int i = msg.data.Find('|');
    CString name = msg.data.Left(i);
    CString size = rightN(msg.data, i + 1);
    i = size.Find('|');
    CString fileMD5 = rightN(size, i + 1);
    size = size.Left(i);
    int fileSize = atoi(size);
    if (fileSize > 1024 * 1024)
        size.Format("%.2f MB", fileSize / 1024.0 / 1024.0);
    else if (fileSize > 1024)
        size.Format("%.2f KB", fileSize / 1024.0);
    else
        size.Format("%d 字节", fileSize);
    if (pDlg->rf.isRecving()) {
        MessageBox(hWnd, '[' + msg.fromUser + "] 给你发来文件：\n文件名：" + name + "\n文件大小：" + size +
                   "\n当前正在接收文件，请等待当前文件接收完再接收其他文件或终止当前文件接收", "温馨提示", 0);
        return;
    }
    GET_WRITE(pDlg->fileTimeOut, "fileTimeOut", "2000");
    if (MessageBox(hWnd, '[' + msg.fromUser + "] 给你发来文件：\n文件名：" + name + "\n文件大小：" + size + "\n是否同意接收？",
                   "温馨提示", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        CString fmt = "*" + name.Right(name.GetLength() - name.ReverseFind('.'));
        CFileDialog dlg(false, 0, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, fmt + "|" + fmt + "|All Files(*.*)|*.*||");
        if (dlg.DoModal() == IDOK) {
            pDlg->fileUser = msg.fromUser;
            pDlg->pSock->SendMSG(msg.join("0", TYPE[AskFileData], "", "", pDlg->fileUser), 0);
            pDlg->rf.init(dlg.GetPathName(), fileSize, fileMD5);
            pDlg->SetTimer(2, pDlg->fileTimeOut, 0);
            pDlg->modifyStatus("准备接收文件！");
        } else
            pDlg->pSock->SendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], "", "", msg.fromUser), 0);
    } else
        pDlg->pSock->SendMSG(msg.join(TYPE[File_NO], TYPE[AskFileData], "", "", msg.fromUser), 0);
}

CString getLastErrorStr()
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                  | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                  , 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
                  , (LPTSTR)&lpMsgBuf, 0, 0);
    CString errStr = (LPCTSTR)lpMsgBuf;
    LocalFree(lpMsgBuf);
    return errStr;
}
