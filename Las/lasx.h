/*
* Copyright (c) 2020, 上海普勒斯道路交通技术有限公司
* All rights reserved.
*
* 文件名称：lasx-1.0.0.0.h
* 文件标识：见配置管理计划书
* 摘要：采用文件映射技术快速读取超大（>4GB）las文件内容
*
* 当前版本：1.0.0.0
* 作者：邹政
* 开始日期：2020年11月24日
* 完成日期：2020年00月00日
*/

#ifndef LASX_H
#define LASX_H
#pragma once
#include <windows.h>
#include <string>
#include <iostream>
//CString的头文件
//引入CString主要是方便字符串与基本类型互转并控制精度，以及同时适应MBCS和UNICODE编码字符串
#include <atlstr.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long llong;

namespace lasx_x
{
	//Public Header Block结构体
	typedef struct PublicHeaderBlock
	{
		char file_signature[4];
		char file_source_id;
		char global_encoding;
	} PUBLIC_HEADER_BLOCK;

	class lasx
	{
	public:
		lasx(const char *fPath);
		~lasx();

	private:
		//文件句柄
		HANDLE h_file;
		//文件映射句柄
		HANDLE h_file_mapping;
		//文件首指针
		char *m_file;
		//当前文件指针
		char *p_file;

		//文件字节数
		__int64 file_size;

	};
}

#endif