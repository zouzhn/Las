/*
* Copyright (c) 2020, 上海普勒斯道路交通技术有限公司
* All rights reserved.
*
* 文件名称：lvxx-1.0.0.7.h
* 文件标识：见配置管理计划书
* 摘要：采用文件映射技术快速读取超大（>4GB）lvx文件内容
* 更新内容：
* 1、改进按帧索引检索的函数MoveToFrame()执行效率：初始化4个指针位置，以此提高对lvx帧数据的检索效率
* 2、改进按整秒索引检索的函数执行效率：获取每个整秒的起始帧、起始包、结束帧和结束包地址，
* 之前的MoveToTime(llong index)现已弃用，推荐使用效率更高的MoveToSecond(llong index)
* 3、加入时间索引方式3，该方式的检索效率较方式1、2有了极大的提升
* 4、优化vector变量定义位置。测试发现，vector变量的定义是十分耗时的，所以不建议在for循环中定义，
* 可以定义到循环外面。总的来说vector的优化效果仍然不够理想
*
* 当前版本：1.0.0.7 (X)
* 作者：邹政
* 开始日期：2020年10月22日
* 完成日期：2020年11月15日
*
* 取代版本：1.0.0.6 (X)
* 原作者：邹政
* 完成日期：2020年10月10日
*/

//#undef UNICODE
#ifndef LVXX_H
#define LVXX_H
#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

//CString的头文件
//引入CString主要是方便字符串与基本类型互转并控制精度，以及同时适应MBCS和UNICODE编码字符串
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
//注：const llong *&arr和const llongp &arr不是等价的，前者在调用时会出现语法错误，
//而后者不会，所以后者才是正确的书写方式，其中typedef llong * llongp
typedef llong * llongp; //可以在自定义类型基础之上继续自定义类型

namespace lvx_x
{
	/*-----------------------------------------------------------
							.lvx文件数据结构
		   注意：结构体的字节数并非是结构体中每个成员的字节数相加
	-----------------------------------------------------------*/
	//Public Header Block结构体
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

	//Private Header Block结构体
	typedef struct PrivateHeaderBlock
	{
		uint frame_duration;
		uchar device_count;
	} PRIVATE_HEADER_BLOCK;
	const ushort PRIVATE_HEADER_BLOCK_SIZE = 5;

	//Devices Info Block结构体
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

	//Point结构体：0-6共7种类型
	typedef struct Point_0 //每个包100个点
	{
		int x;
		int y;
		int z;
		uchar reflectivity;
	} POINT_0;
	const ushort POINT_0_SIZE = 13;

	typedef struct Point_1 //每个包100个点
	{
		int depth;
		ushort theta;
		ushort phi;
		uchar reflectivity;
	} POINT_1;
	const ushort POINT_1_SIZE = 9;

	typedef struct Point_2 //每个包96个点
	{
		int x; //单位：mm
		int y; //单位：mm
		int z; //单位：mm
		uchar reflectivity; //0-255整数
		uchar tag;
	} POINT_2;
	const ushort POINT_2_SIZE = 14;

	typedef struct Point_3 //每个包96个点
	{
		int depth;
		ushort theta;
		ushort phi;
		uchar reflectivity;
		uchar tag;
	} POINT_3;
	const ushort POINT_3_SIZE = 10;

	typedef struct Point_4 //每个包48个点
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

	typedef struct Point_5 //每个包48个点
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

	typedef struct Point_6 //每个包1个点
	{
		float gyro_x;
		float gyro_y;
		float gyro_z;
		float acc_x;
		float acc_y;
		float acc_z;
	} POINT_6;
	const ushort POINT_6_SIZE = 24;

	//Package结构体
	typedef struct Package
	{
		uchar device_index;
		uchar version;
		uchar slot_id;
		uchar lidar_id;
		uchar reserved;
		uint status_code; //或者叫error_code
		uchar timestamp_type;
		uchar data_type;
		llong timestamp; //pdf中是uchar[8]类型，这里为了展示方便，直接写成long long类型
		vector<POINT_2> point_2;
	} PACKAGE;

	//Frame Header结构体
	typedef struct FrameHeader
	{
		llong current_offset;
		llong next_offset;
		llong frame_index;
	} FRAME_HEADER;
	const ushort FRAME_HEADER_SIZE = 24;

	//Frame结构体
	typedef struct Frame
	{
		FRAME_HEADER frame_header;
		vector<PACKAGE> package;
	} FRAME;

	//帧指针位置结构体
	typedef struct FramePointerPosition
	{
		//首帧指针
		char *begin;
		llong beginIndex; //对应的帧索引（从0开始）
		//尾帧指针
		char *end;
		llong endIndex;
		//中间帧指针
		char *middle;
		llong middleIndex;
		//当前帧指针
		char *current;
		llong currentIndex;
	} FPP;

	//整秒指针位置结构体变量
	typedef struct IntegerSecondFramePointerPosition
	{
		//整秒开始帧指针
		char *beginFrame;
		//整秒开始帧中开始包指针
		char *beginPackage;
		//整秒结束帧指针
		char *endFrame;
		//整秒结束帧中结束包指针
		char *endPackage;
		//对应的秒索引（从0开始）
		//能够表示的整秒数目最大为65536，也就是大约18个小时
		//定义成ushort而非int和llong也是为了节约运行内存
		ushort secondIndex;
	} ISFPP;

	//POINT_2与timestamp的结合
	typedef struct POINT_2ANDTIMESTAMP
	{
		llong timestamp; //pdf中是uchar[8]类型，这里为了展示方便，直接写成long long类型
		vector<POINT_2> point_2;
	} P_2AT;

	/*-------------------------------------------------------------------
						.lvx文件解析类（引入文件映射技术）
	-------------------------------------------------------------------*/
	class lvxx
	{
	public:

		//构造函数
		lvxx(const char *pfile);
		~lvxx();

		//获取文件字节数
		//文件最末的指针一定是指向文件结束标志（EOF字符）的，但总的字节数却并没有包括EOF占用的字节（亲测）
		__int64 GetFileLength();
		
		//获取帧数目（根据lvx文件数据结构pdf可知，一帧必定为50ms）
		llong GetFrameCount();
		//获取整秒数目
		llong GetSecondCount();
		//获取整个lvx文件中的点数目
		llong GetPointsCount();

		//获取PUBLIC_HEADER_BLOCK
		PUBLIC_HEADER_BLOCK GetPublicHeaderBlock();
		//获取PRIVATE_HEADER_BLOCK
		PRIVATE_HEADER_BLOCK GetPrivateHeaderBlock();
		//获取DEVICES_INFO_BLOCK
		DEVICES_INFO_BLOCK GetDevicesInfoBlock();
		
		//将文件指针移动到指定索引帧首地址并返回指针值
		char *MoveToFrame(llong index);
		//将文件指针移动到指定索引整秒内每个包中首点的首地址（此函数从1.0.0.7版本开始不再推荐使用）
		//推荐使用MoveToSecond函数
		vector<char *> MoveToTime(llong index);
		//将文件指针移动到指定索引整秒内每个包的首地址并保存（注意：这里只取datatype为2的包）
		vector<char *> MoveToSecond(llong index);

		//按帧/秒索引提取数据
		FRAME frame; //按帧索引提取帧数据
		llong point_count_in_frame; //frame中的点数目
		vector<PACKAGE> data; //按秒索引提取数据（方式1）
		llong point_count_in_data; //data中的点数目
		vector<POINT_2> point; //按秒索引提取数据（方式2）
		llong point_count; //point中的点数目
		vector<P_2AT> p_2at; //按秒索引提取数据（方式3）
		llong pointsCount; //p_2at中的点数目

		//mode：0-帧索引方式；1-时间索引方式1；2-时间索引方式2；3-时间索引方式3
		//mode为0时返回：帧索引
		//mode为1时返回：最后一个包大概在哪个索引帧里面（为i或者i-1）
		//mode为2时返回：点的数目
		//mode为3时返回：点的数目
		//返回值若为-1，则表示mode输入错误
		llong ExtractData(llong index, const int mode);

	private:

		/*---------------------------------------
						数据成员
		---------------------------------------*/
		//文件句柄
		HANDLE h_file;
		//文件映射句柄
		HANDLE h_file_mapping;
		//文件首指针
		char *m_file;
		//当前文件指针
		char *p_file;

		//第一帧首地址
		char *first_frame;
		//最后一帧首地址
		char *last_frame;

		//帧指针位置结构体变量
		//适用于按帧索引方式
		FPP fpp;

		//整秒指针位置结构体变量
		//适用于按秒索引方式
		//在初始化整秒数变量的时候初始化isfpp
		std::vector<ISFPP> isfpps;
		//存放整秒中每个包的位置
		vector<char *> p_package;
		P_2AT temp_p2at;

		//文件字节数
		__int64 file_size;

		//文件中总的帧数目
		llong frame_count;
		//整秒数目
		llong second_count;
		//整个lvx文件中的点数目
		llong points_count_in_lvx;

		//结构体变量
		PUBLIC_HEADER_BLOCK public_header_block;
		PRIVATE_HEADER_BLOCK private_header_block;
		DEVICES_INFO_BLOCK devices_info_block;

		POINT_2 point_2;
		PACKAGE package;
		FRAME_HEADER frame_header;

		/*---------------------------------------
						成员函数
		---------------------------------------*/
		//设置PUBLIC_HEADER_BLOCK
		void SetPublicHeaderBlock();
		//获取PRIVATE_HEADER_BLOCK
		void SetPrivateHeaderBlock();
		//获取DEVICES_INFO_BLOCK
		void SetDevicesInfoBlock();

		//设置第一帧首地址
		void SetFirstFrame();
		//设置最后一帧首地址
		//void SetLastFrame();
		//设置文件总帧数目（顺便初始化最后一帧的首地址）
		void SetFrameCount();
		//设置整秒数目（顺便初始化isfpps）
		void SetSecondCount();

		//初始化帧指针结构体变量
		void InitializeFPP();

		//求绝对值
		llong Abs(const llong &a, const llong &b);

		//将帧索引与各个预设位置比较并返回最近的位置指针
		//index-帧索引
		char *CompareWithFPP(const llong &index);

		//清空包
		void ClearFramePackage();

		//关闭所有文件句柄
		void CloseAllFileHandles();
	};
}

#endif // LVXX_H

