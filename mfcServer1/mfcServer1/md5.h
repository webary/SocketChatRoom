#ifndef __MD5_H__
#define __MD5_H__
#pragma message("--温馨提示：请将运行库设置为“多线程调试”方式")
#define dynamic_link 0

#if dynamic_link
	#ifdef   FILE_DLL_EXPORT
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#pragma comment(lib,"md5.lib")
	#define DLL_EXPORT
#endif

#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

class DLL_EXPORT MD5 {
public:
    MD5();
    virtual ~MD5();
    bool fileMd5(char *pMd5, const char *pFileName);
	bool strMd5 (char *pMd5, const char *str);

private:
    unsigned long int state[4];		/* state (ABCD) */
    unsigned long int count[2];		/* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];       /* input buffer */
    unsigned char PADDING[64];		/* What? */

private:
    void MD5Init ();
    void MD5Update( unsigned char *input, unsigned int inputLen);
    void MD5Final (unsigned char digest[16]);
    void MD5Transform (unsigned long int state[4], unsigned char block[64]);
    void MD5_memcpy (unsigned char* output, unsigned char* input,unsigned int len);
    void Encode (unsigned char *output, unsigned long int *input,unsigned int len);
    void Decode (unsigned long int *output, unsigned char *input, unsigned int len);
    void MD5_memset (unsigned char* output,int value,unsigned int len);
};
//返回一个字符串的md5值
DLL_EXPORT const char* getStrMd5(char* md5_value, const char* str);

#endif
