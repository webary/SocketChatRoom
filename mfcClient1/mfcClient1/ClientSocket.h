#pragma once
#include "afxsock.h"

#define MBox(s) MessageBox(s,"温馨提示")
#define MBox2(s1,s2) MessageBox(s1,s2)
#define elif else if
#define FOR(ii,start,end) for(ii=start;ii<end;++ii)

extern CString TYPE[30];	//消息类型的定义
extern CString STR[5];		//连接消息的各个部分

inline CString rightN(CString str,int n)
{
	return str.Right(str.GetLength()-n);
}

	//数据缓冲区大小
#define DATA_BUF_SIZE	(8*1024)
	//文件拆包后每个包最大字节数
#define PACKAGE_SIZE	(512)
	//最大数据包数目
#define MAX_PACKAGE_NUM	(16*1024)
class CmfcClient1Dlg;
class CClientSocket : public CSocket
{
public:
	struct MyMsg
	{
		CString userId;		//用户名
		CString pw;			//密码
		CString type;		//消息类型
		CString fromUser;	//消息来自
		CString toUser;		//消息去向
		CString data;		//消息内容
		CString load(CString str,bool OLMsg=0)
		{
			CString tempStr[6] = {""};
			int index=0, i;
			FOR(index,0,5)
			{
				i = str.Find(STR[index]);
				tempStr[index] = str.Left(i);
				str = rightN(str,i+3);
				if(str == "")
					break;
			}
			tempStr[5] = str;
			i = str.Find(STR[0]);
			if(i!=-1 && OLMsg)
			{
				tempStr[5] = str.Left(i-1);
				str = rightN(str,i-1);
			}
			index=0;
			userId = tempStr[index++];
			pw = tempStr[index++];
			fromUser = tempStr[index++];
			toUser = tempStr[index++];
			type = tempStr[index++];
			data = tempStr[index++];
			return str;
		}
		const CString join(CString _data="",CString _type="",CString _user="",CString _from="",CString _to="",CString _pw="") const
		{
			if(_user=="")
				_user = userID;//userId;
			//用户名+密码+来自+去向+类型+内容
			return _user+STR[0] + _pw+STR[1] + _from+STR[2] + _to+STR[3] +_type +STR[4] + _data;
		}
	};
public:	
	MyMsg mymsg;
	//CString user, msg;	//临时用户名和消息内容
	static CString userID;	//该用户的用户名
	//CString fromUser;	//从该用户发来的消息
	LONG	IDLen;		//ID的长度
	int errorCode;		//错误码

	CmfcClient1Dlg* pDlg;
	HWND hWnd;
	CClientSocket(CString _user);
	~CClientSocket();
	virtual void OnReceive(int nErrorCode);// 重写接收函数，通过类向导生成
	void SendMSG(CString send,bool upEvent=1);// 发送函数，用于发送数据给服务器
	int getError();	//得到最后一个错误代码
	void updateEvent(CString showMsg, CString from="服务器:",bool reset = 0,int timeFMT = 2);
	void fileSend(MyMsg& msg);	//接收到发送文件的请求时
};

#include "CXXFStream.hpp"

#include "RecvFile.hpp"
