/*
* Copyright (c) 2020, �Ϻ�����˹��·��ͨ�������޹�˾
* All rights reserved.
*
* �ļ����ƣ�lasx-1.0.0.0.h
* �ļ���ʶ�������ù���ƻ���
* ժҪ�������ļ�ӳ�似�����ٶ�ȡ����>4GB��las�ļ�����
*
* ��ǰ�汾��1.0.0.0
* ���ߣ�����
* ��ʼ���ڣ�2020��11��24��
* ������ڣ�2020��00��00��
*/

#ifndef LASX_H
#define LASX_H
#pragma once
#include <windows.h>
#include <string>
#include <iostream>
//CString��ͷ�ļ�
//����CString��Ҫ�Ƿ����ַ�����������ͻ�ת�����ƾ��ȣ��Լ�ͬʱ��ӦMBCS��UNICODE�����ַ���
#include <atlstr.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long llong;

namespace lasx_x
{
	//Public Header Block�ṹ��
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
		//�ļ����
		HANDLE h_file;
		//�ļ�ӳ����
		HANDLE h_file_mapping;
		//�ļ���ָ��
		char *m_file;
		//��ǰ�ļ�ָ��
		char *p_file;

		//�ļ��ֽ���
		__int64 file_size;

	};
}

#endif