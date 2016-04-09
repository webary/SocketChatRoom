#ifndef _MYMSG_H_
#define _MYMSG_H_

#include "afxstr.h"

extern const CString TYPE[30];  //消息类型
extern const char seperator[3]; //消息各个字段的分隔符

//负责消息解析和封装的类，定义为struct让所有数据成员都公开
struct MyMsg
{
    CString userId;     //用户名
    CString pw;         //密码
    CString type;       //消息类型
    CString fromUser;   //消息来自
    CString toUser;     //消息去向
    CString data;       //消息内容

    explicit MyMsg(CString str = "");
    //载入消息，即解析消息。OLMsg标记是否是离线消息
    CString load(CString str, bool OLMsg = 0);
    //连接消息各个部分，即封装消息
    CString join(CString _data = "", CString _type = "", CString _user = "", CString _from = "", CString _to = "", CString _pw = "") const;
    //返回去除str前n个字符之后的右边剩余的子串
    static CString rightN(const CString &str, int n) {
        return str.GetLength() > n ? str.Right(str.GetLength() - n) : "";
    }
};

#endif // !_MYMSG_H_
