#pragma once
#ifndef _SERVERSOCKET_H_
#define _SERVERSOCKET_H_

#include <afxsock.h>

#define DATA_BUF_SIZE   (32*1024) //数据缓冲区大小
#define PACKAGE_SIZE    (512)     //文件拆包后每个包最大字节数
#define MAX_PACKAGE_NUM (16*1024) //最大数据包数目

#define MBox(s) MessageBox(s,"温馨提示")
#define MBox2(s1,s2) MessageBox(s1,s2)
#define elif else if
#define FOR(ii,start,end) for(ii=start;ii<end;++ii)

extern CString TYPE[30];
extern CString STR[5];      //连接消息的各个部分

//负责消息解析和封装的类，定义为struct让所有数据成员都公开
struct MyMsg
{
    CString userId;     //用户名
    CString pw;         //密码
    CString type;       //消息类型
    CString fromUser;   //消息来自
    CString toUser;     //消息去向
    CString data;       //消息内容

    explicit MyMsg(const CString str = "") {
        if (str != "")
            load(str);
    }
    //载入消息，即解析消息。OLMsg标记是否是离线消息
    CString load(CString str, bool OLMsg = 0) {
        CString tempStr[6] = { "" };
        int index = 0, i;
        FOR(index, 0, 5) {
            i = str.Find(STR[index]);
            tempStr[index] = str.Left(i);
            str = rightN(str, i + 3);
            if (str == "") {
                break;
            }
        }
        tempStr[5] = str;
        i = str.Find(STR[0]);
        if (i != -1 && OLMsg) {
            tempStr[5] = str.Left(i - 1);
            str = rightN(str, i - 1);
        }
        index = 0;
        userId = tempStr[index++];
        pw = tempStr[index++];
        fromUser = tempStr[index++];
        toUser = tempStr[index++];
        type = tempStr[index++];
        data = tempStr[index++];
        return str;
    }
    //连接消息各个部分，即封装消息
    const CString join(CString _data = "", CString _type = "", CString _user = "", CString _from = "", CString _to = "", CString _pw = "") const {
        if (_user == "")
            _user = userId;
        //用户名+密码+来自+去向+类型+内容
        return _user + STR[0] + _pw + STR[1] + _from + STR[2] + _to + STR[3] + _type + STR[4] + _data;
    }
    //返回去除str前n个字符之后的右边剩余的子串
    static CString rightN(CString str, int n) {
        return str.Right(str.GetLength() - n);
    }
};

class CmfcServer1Dlg; //类声明,因为下面定义了一个主窗口指针
class CServerSocket : public CSocket
{
public:
    CServerSocket(CmfcServer1Dlg* m_pDlg);

    virtual void OnAccept(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
public:
    CmfcServer1Dlg* m_pDlg;
};

#endif //_SERVERSOCKET_H_
