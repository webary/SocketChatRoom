#ifndef _RECVFILE_HPP_
#define _RECVFILE_HPP_

#include <fstream>
#include <afxstr.h> //CString

extern char packageData[MAX_PACKAGE_NUM][2 * PACKAGE_SIZE + 1];

class RecvFile
{
    bool recving;
    CString fileNewName;
    long packageNum;    //数据包的数目
    long packageRecv;   //已接收的数据包数目
    long fileLength;    //文件大小
    CString fileMD5;    //文件的md5值
    unsigned timeStart; //开始文件传输的时间
    CString timeMsg;    //耗费时间提示
    CString pack_rec;
    bool success;       //接收成功
public:
    RecvFile() {
        packageRecv = 0;
        recving = 0;
    }
    void init(const CString& name, int size, const CString& md5) {
        timeStart = clock();
        fileNewName = name;
        fileLength = size;
        fileMD5 = md5;
        packageNum = (size + PACKAGE_SIZE - 1) / PACKAGE_SIZE;
        packageRecv = 0;
        clearPackData();
        recving = 1;
        std::ofstream out(fileNewName);
        out.close();
    }
    void clearPackData() {
        memset(packageData, 0, sizeof(packageData));
    }
    void recvEnd(bool clear = 1) {  //clear标记是否清空数据包内容
        if (packageNum > packageRecv) {  //文件接收失败
            DeleteFile(fileNewName);
        }
        if(clear)
            clearPackData();
        packageNum = packageRecv = 0;
        recving = 0;
    }
    const bool& isRecving() {
        return recving;
    }
    const CString& getPackRecv() {
        pack_rec.Format("%ld", packageRecv);
        return pack_rec;
    }
    long getPackNum() {
        return packageNum;
    }
    const CString& getFileName() {
        return fileNewName;
    }
    void setPackNum(long num) {
        packageNum = num;
    }
    void addPacketage(const char *data);
private:
    void saveFile(int useTime);
};

#endif //_RECVFILE_HPP_
