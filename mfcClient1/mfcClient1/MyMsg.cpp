#include "stdafx.h"
#include "MyMsg.h"

//消息类型
const CString TYPE[30] = { TYPE_ChatMsg , TYPE_Server_is_closed , TYPE_UserList , TYPE_OnlineState , TYPE_FileSend , TYPE_FileData , TYPE_AskFileData , TYPE_File_NO , TYPE_File_Over , TYPE_File_Fail , TYPE_LoginFail , TYPE_UserIsOnline , TYPE_OfflineMsg , TYPE_AllUser , TYPE_AddUserList , TYPE_I_am_online , TYPE_Logout , TYPE_Login , TYPE_Register , TYPE_Status };
//每两个字段间的分隔符
const char seperator[3] = { 0x1F, 0x7C };

MyMsg::MyMsg(CString str) {
    if (str != "")
        load(str);
}
//载入消息，即解析消息。OLMsg标记是否是离线消息
CString MyMsg::load(CString str, bool OLMsg /* = 0 */) {
    CString tempStr[6] = { "" };
    int index = 0, i;
    for (index = 0; index < 5; ++index) {
        i = str.Find(seperator);
        tempStr[index] = str.Left(i);
        str = rightN(str, i + 2);
        if (str == "")
            break;
    }
    tempStr[5] = str;
    //处理离线消息
    i = str.Find(seperator);
    if (i != -1 && OLMsg) {
        tempStr[5] = str.Left(i - 1);
        str = rightN(str, i - 1);
    }
    //将消息的各个字段存储到对应变量中
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
CString MyMsg::join(CString _data, CString _type, CString _user, CString _from, CString _to, CString _pw) const
{
    if (_user == "")
        _user = userId;
    //用户名+密码+来自+去向+类型+内容
    return _user + seperator + _pw + seperator + _from + seperator
        + _to + seperator + _type + seperator + _data;
}
