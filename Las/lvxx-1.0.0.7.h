/*
* Copyright (c) 2020, �Ϻ�����˹��·��ͨ�������޹�˾
* All rights reserved.
*
* �ļ����ƣ�lvxx-1.0.0.7.h
* �ļ���ʶ�������ù���ƻ���
* ժҪ�������ļ�ӳ�似�����ٶ�ȡ����>4GB��lvx�ļ�����
* �������ݣ�
* 1���Ľ���֡���������ĺ���MoveToFrame()ִ��Ч�ʣ���ʼ��4��ָ��λ�ã��Դ���߶�lvx֡���ݵļ���Ч��
* 2���Ľ����������������ĺ���ִ��Ч�ʣ���ȡÿ���������ʼ֡����ʼ��������֡�ͽ�������ַ��
* ֮ǰ��MoveToTime(llong index)�������ã��Ƽ�ʹ��Ч�ʸ��ߵ�MoveToSecond(llong index)
* 3������ʱ��������ʽ3���÷�ʽ�ļ���Ч�ʽϷ�ʽ1��2���˼��������
* 4���Ż�vector��������λ�á����Է��֣�vector�����Ķ�����ʮ�ֺ�ʱ�ģ����Բ�������forѭ���ж��壬
* ���Զ��嵽ѭ�����档�ܵ���˵vector���Ż�Ч����Ȼ��������
*
* ��ǰ�汾��1.0.0.7 (X)
* ���ߣ�����
* ��ʼ���ڣ�2020��10��22��
* ������ڣ�2020��11��15��
*
* ȡ���汾��1.0.0.6 (X)
* ԭ���ߣ�����
* ������ڣ�2020��10��10��
*/

//#undef UNICODE
#ifndef LVXX_H
#define LVXX_H
#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

//CString��ͷ�ļ�
//����CString��Ҫ�Ƿ����ַ�����������ͻ�ת�����ƾ��ȣ��Լ�ͬʱ��ӦMBCS��UNICODE�����ַ���
#include <atlstr.h>

#define POINT_NUM_IN_EACH_PACKAGE 96
#define FRAME_INDEX 0
#define TIME_INDEX_1 1
#define TIME_INDEX_2 2
#define TIME_INDEX_3 3

using namespace std;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long llong;
//ע��const llong *&arr��const llongp &arr���ǵȼ۵ģ�ǰ���ڵ���ʱ������﷨����
//�����߲��ᣬ���Ժ��߲�����ȷ����д��ʽ������typedef llong * llongp
typedef llong * llongp; //�������Զ������ͻ���֮�ϼ����Զ�������

namespace lvx_x
{
	/*-----------------------------------------------------------
							.lvx�ļ����ݽṹ
		   ע�⣺�ṹ����ֽ��������ǽṹ����ÿ����Ա���ֽ������
	-----------------------------------------------------------*/
	//Public Header Block�ṹ��
	typedef struct PublicHeaderBlock
	{
		char file_signature[16];
		char version_a;
		char version_b;
		char version_c;
		char version_d;
		uint magic_code;
	} PUBLIC_HEADER_BLOCK;
	const ushort PUBLIC_HEADER_BLOCK_SIZE = 24;

	//Private Header Block�ṹ��
	typedef struct PrivateHeaderBlock
	{
		uint frame_duration;
		uchar device_count;
	} PRIVATE_HEADER_BLOCK;
	const ushort PRIVATE_HEADER_BLOCK_SIZE = 5;

	//Devices Info Block�ṹ��
	typedef struct DevicesInfoBlock
	{
		char lidar_sn_code[16];
		char hub_sn_code[16];
		uchar device_index;
		uchar device_type;
		uchar extrinsic_enable;
		float roll;
		float pitch;
		float yaw;
		float x;
		float y;
		float z;
	} DEVICES_INFO_BLOCK;
	const ushort DEVICES_INFO_BLOCK_SIZE = 59;

	//Point�ṹ�壺0-6��7������
	typedef struct Point_0 //ÿ����100����
	{
		int x;
		int y;
		int z;
		uchar reflectivity;
	} POINT_0;
	const ushort POINT_0_SIZE = 13;

	typedef struct Point_1 //ÿ����100����
	{
		int depth;
		ushort theta;
		ushort phi;
		uchar reflectivity;
	} POINT_1;
	const ushort POINT_1_SIZE = 9;

	typedef struct Point_2 //ÿ����96����
	{
		int x; //��λ��mm
		int y; //��λ��mm
		int z; //��λ��mm
		uchar reflectivity; //0-255����
		uchar tag;
	} POINT_2;
	const ushort POINT_2_SIZE = 14;

	typedef struct Point_3 //ÿ����96����
	{
		int depth;
		ushort theta;
		ushort phi;
		uchar reflectivity;
		uchar tag;
	} POINT_3;
	const ushort POINT_3_SIZE = 10;

	typedef struct Point_4 //ÿ����48����
	{
		int x1;
		int y1;
		int z1;
		uchar reflectivity1;
		uchar tag1;
		int x2;
		int y2;
		int z2;
		uchar reflectivity2;
		uchar tag2;
	} POINT_4;
	const ushort POINT_4_SIZE = 28;

	typedef struct Point_5 //ÿ����48����
	{
		int theta;
		int phi;
		int depth1;
		uchar reflectivity1;
		uchar tag1;
		int depth2;
		uchar reflectivity2;
		uchar tag2;
	} POINT_5;
	const ushort POINT_5_SIZE = 20;

	typedef struct Point_6 //ÿ����1����
	{
		float gyro_x;
		float gyro_y;
		float gyro_z;
		float acc_x;
		float acc_y;
		float acc_z;
	} POINT_6;
	const ushort POINT_6_SIZE = 24;

	//Package�ṹ��
	typedef struct Package
	{
		uchar device_index;
		uchar version;
		uchar slot_id;
		uchar lidar_id;
		uchar reserved;
		uint status_code; //���߽�error_code
		uchar timestamp_type;
		uchar data_type;
		llong timestamp; //pdf����uchar[8]���ͣ�����Ϊ��չʾ���㣬ֱ��д��long long����
		vector<POINT_2> point_2;
	} PACKAGE;

	//Frame Header�ṹ��
	typedef struct FrameHeader
	{
		llong current_offset;
		llong next_offset;
		llong frame_index;
	} FRAME_HEADER;
	const ushort FRAME_HEADER_SIZE = 24;

	//Frame�ṹ��
	typedef struct Frame
	{
		FRAME_HEADER frame_header;
		vector<PACKAGE> package;
	} FRAME;

	//ָ֡��λ�ýṹ��
	typedef struct FramePointerPosition
	{
		//��ָ֡��
		char *begin;
		llong beginIndex; //��Ӧ��֡��������0��ʼ��
		//βָ֡��
		char *end;
		llong endIndex;
		//�м�ָ֡��
		char *middle;
		llong middleIndex;
		//��ǰָ֡��
		char *current;
		llong currentIndex;
	} FPP;

	//����ָ��λ�ýṹ�����
	typedef struct IntegerSecondFramePointerPosition
	{
		//���뿪ʼָ֡��
		char *beginFrame;
		//���뿪ʼ֡�п�ʼ��ָ��
		char *beginPackage;
		//�������ָ֡��
		char *endFrame;
		//�������֡�н�����ָ��
		char *endPackage;
		//��Ӧ������������0��ʼ��
		//�ܹ���ʾ��������Ŀ���Ϊ65536��Ҳ���Ǵ�Լ18��Сʱ
		//�����ushort����int��llongҲ��Ϊ�˽�Լ�����ڴ�
		ushort secondIndex;
	} ISFPP;

	//POINT_2��timestamp�Ľ��
	typedef struct POINT_2ANDTIMESTAMP
	{
		llong timestamp; //pdf����uchar[8]���ͣ�����Ϊ��չʾ���㣬ֱ��д��long long����
		vector<POINT_2> point_2;
	} P_2AT;

	/*-------------------------------------------------------------------
						.lvx�ļ������ࣨ�����ļ�ӳ�似����
	-------------------------------------------------------------------*/
	class lvxx
	{
	public:

		//���캯��
		lvxx(const char *pfile);
		~lvxx();

		//��ȡ�ļ��ֽ���
		//�ļ���ĩ��ָ��һ����ָ���ļ�������־��EOF�ַ����ģ����ܵ��ֽ���ȴ��û�а���EOFռ�õ��ֽڣ��ײ⣩
		__int64 GetFileLength();
		
		//��ȡ֡��Ŀ������lvx�ļ����ݽṹpdf��֪��һ֡�ض�Ϊ50ms��
		llong GetFrameCount();
		//��ȡ������Ŀ
		llong GetSecondCount();
		//��ȡ����lvx�ļ��еĵ���Ŀ
		llong GetPointsCount();

		//��ȡPUBLIC_HEADER_BLOCK
		PUBLIC_HEADER_BLOCK GetPublicHeaderBlock();
		//��ȡPRIVATE_HEADER_BLOCK
		PRIVATE_HEADER_BLOCK GetPrivateHeaderBlock();
		//��ȡDEVICES_INFO_BLOCK
		DEVICES_INFO_BLOCK GetDevicesInfoBlock();
		
		//���ļ�ָ���ƶ���ָ������֡�׵�ַ������ָ��ֵ
		char *MoveToFrame(llong index);
		//���ļ�ָ���ƶ���ָ������������ÿ�������׵���׵�ַ���˺�����1.0.0.7�汾��ʼ�����Ƽ�ʹ�ã�
		//�Ƽ�ʹ��MoveToSecond����
		vector<char *> MoveToTime(llong index);
		//���ļ�ָ���ƶ���ָ������������ÿ�������׵�ַ�����棨ע�⣺����ֻȡdatatypeΪ2�İ���
		vector<char *> MoveToSecond(llong index);

		//��֡/��������ȡ����
		FRAME frame; //��֡������ȡ֡����
		llong point_count_in_frame; //frame�еĵ���Ŀ
		vector<PACKAGE> data; //����������ȡ���ݣ���ʽ1��
		llong point_count_in_data; //data�еĵ���Ŀ
		vector<POINT_2> point; //����������ȡ���ݣ���ʽ2��
		llong point_count; //point�еĵ���Ŀ
		vector<P_2AT> p_2at; //����������ȡ���ݣ���ʽ3��
		llong pointsCount; //p_2at�еĵ���Ŀ

		//mode��0-֡������ʽ��1-ʱ��������ʽ1��2-ʱ��������ʽ2��3-ʱ��������ʽ3
		//modeΪ0ʱ���أ�֡����
		//modeΪ1ʱ���أ����һ����������ĸ�����֡���棨Ϊi����i-1��
		//modeΪ2ʱ���أ������Ŀ
		//modeΪ3ʱ���أ������Ŀ
		//����ֵ��Ϊ-1�����ʾmode�������
		llong ExtractData(llong index, const int mode);

	private:

		/*---------------------------------------
						���ݳ�Ա
		---------------------------------------*/
		//�ļ����
		HANDLE h_file;
		//�ļ�ӳ����
		HANDLE h_file_mapping;
		//�ļ���ָ��
		char *m_file;
		//��ǰ�ļ�ָ��
		char *p_file;

		//��һ֡�׵�ַ
		char *first_frame;
		//���һ֡�׵�ַ
		char *last_frame;

		//ָ֡��λ�ýṹ�����
		//�����ڰ�֡������ʽ
		FPP fpp;

		//����ָ��λ�ýṹ�����
		//�����ڰ���������ʽ
		//�ڳ�ʼ��������������ʱ���ʼ��isfpp
		std::vector<ISFPP> isfpps;
		//���������ÿ������λ��
		vector<char *> p_package;
		P_2AT temp_p2at;

		//�ļ��ֽ���
		__int64 file_size;

		//�ļ����ܵ�֡��Ŀ
		llong frame_count;
		//������Ŀ
		llong second_count;
		//����lvx�ļ��еĵ���Ŀ
		llong points_count_in_lvx;

		//�ṹ�����
		PUBLIC_HEADER_BLOCK public_header_block;
		PRIVATE_HEADER_BLOCK private_header_block;
		DEVICES_INFO_BLOCK devices_info_block;

		POINT_2 point_2;
		PACKAGE package;
		FRAME_HEADER frame_header;

		/*---------------------------------------
						��Ա����
		---------------------------------------*/
		//����PUBLIC_HEADER_BLOCK
		void SetPublicHeaderBlock();
		//��ȡPRIVATE_HEADER_BLOCK
		void SetPrivateHeaderBlock();
		//��ȡDEVICES_INFO_BLOCK
		void SetDevicesInfoBlock();

		//���õ�һ֡�׵�ַ
		void SetFirstFrame();
		//�������һ֡�׵�ַ
		//void SetLastFrame();
		//�����ļ���֡��Ŀ��˳���ʼ�����һ֡���׵�ַ��
		void SetFrameCount();
		//����������Ŀ��˳���ʼ��isfpps��
		void SetSecondCount();

		//��ʼ��ָ֡��ṹ�����
		void InitializeFPP();

		//�����ֵ
		llong Abs(const llong &a, const llong &b);

		//��֡���������Ԥ��λ�ñȽϲ����������λ��ָ��
		//index-֡����
		char *CompareWithFPP(const llong &index);

		//��հ�
		void ClearFramePackage();

		//�ر������ļ����
		void CloseAllFileHandles();
	};
}

#endif // LVXX_H

