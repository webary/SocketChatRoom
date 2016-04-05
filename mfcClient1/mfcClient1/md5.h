/******************************************************************************
 *  Copyright (C) 2000 by Robert Hubley.                                      *
 *  All rights reserved.                                                      *
 *                                                                            *
 *  This software is provided ``AS IS'' and any express or implied            *
 *  warranties, including, but not limited to, the implied warranties of      *
 *  merchantability and fitness for a particular purpose, are disclaimed.     *
 *  In no event shall the authors be liable for any direct, indirect,         *
 *  incidental, special, exemplary, or consequential damages (including, but  *
 *  not limited to, procurement of substitute goods or services; loss of use, *
 *  data, or profits; or business interruption) however caused and on any     *
 *  theory of liability, whether in contract, strict liability, or tort       *
 *  (including negligence or otherwise) arising in any way out of the use of  *
 *  this software, even if advised of the possibility of such damage.         *
 *                                                                            *
 ******************************************************************************
    MD5.H - header file for MD5C.C
    Port to Win32 DLL by Robert Hubley 1/5/2000
    Original Copyright:
        Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
        rights reserved.

        License to copy and use this software is granted provided that it
        is identified as the "RSA Data Security, Inc. MD5 Message-Digest
        Algorithm" in all material mentioning or referencing this software
        or this function.

        License is also granted to make and use derivative works provided
        that such works are identified as "derived from the RSA Data
        Security, Inc. MD5 Message-Digest Algorithm" in all material
        mentioning or referencing the derived work.

        RSA Data Security, Inc. makes no representations concerning either
        the merchantability of this software or the suitability of this
        software for any particular purpose. It is provided "as is"
        without express or implied warranty of any kind.

        These notices must be retained in any copies of any part of this
        documentation and/or software.
*/
/******************************************************************************
 *   2002-4-18 Modified by Liguangyi.                                         *
 *   struct MD5 ==> class MD5.                                                *
 *   Take off the Globals Functions                                           *
 ******************************************************************************
*/
#ifndef _MD5_H_
#define _MD5_H_

//屏蔽vs中针对一些库函数的非必要的安全警告
#define _CRT_SECURE_NO_WARNINGS

//在编译器中调整模式后才能正确编译通过
#pragma message("--温馨提示：请确保已将C/C++ => 代码生成 => 运行库"\
    "设置为“多线程调试(/MTd)”模式")

//设置动态链接属性,设置为0则表示静态链接
#define dynamic_link 0

#if dynamic_link
#   ifdef FILE_DLL_EXPORT
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT __declspec(dllimport)
#   endif
#else
#   pragma comment(lib,"md5.lib")
#   define DLL_EXPORT
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

class DLL_EXPORT MD5
{
public:
    MD5();
    virtual ~MD5();
    bool fileMd5(char *pMd5, const char *pFileName);
    bool strMd5 (char *pMd5, const char *str);

private:
    unsigned long int state[4];     // state (ABCD)
    unsigned long int count[2];     // number of bits, modulo 2^64 (lsb first)
    unsigned char buffer[64];       // input buffer
    unsigned char PADDING[64];      // What?
private:
    void MD5Init();
    void MD5Update(unsigned char *input, unsigned int inputLen);
    void MD5Final(unsigned char digest[16]);
    void MD5Transform(unsigned long int state[4], unsigned char block[64]);
    void MD5_memcpy(unsigned char* output, unsigned char* input,unsigned int len);
    void Encode(unsigned char *output, unsigned long int *input,unsigned int len);
    void Decode(unsigned long int *output, unsigned char *input, unsigned int len);
    void MD5_memset(unsigned char* output,int value,unsigned int len);
};
//全局函数:返回一个字符串str的md5值md5_value
DLL_EXPORT const char* getStrMd5(char* md5_value, const char* str);

#endif //_MD5_H_
