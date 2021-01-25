#include "lvxx-1.0.0.7.h"

//构造函数
lvx_x::lvxx::lvxx(const char *pfile)
{
	//利用CString增强程序移植性（同时适应char和wchar_t类型）
	CString filePath(pfile);

	//打开期望映射的文件（映射可以想象成水面上的倒影，虽然看得见，但不是真实存在的）
	h_file = CreateFile(filePath, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//得到整个文件的大小
	DWORD hi; //高32位
	file_size = GetFileSize(h_file, &hi); //低32位
	file_size |= (((__int64)hi) << 32); //文件大小

	//创建文件映射句柄
	h_file_mapping = CreateFileMapping(h_file, NULL, PAGE_READONLY, NULL, NULL, NULL);

	//映射整个文件
	m_file = (char *)MapViewOfFile(h_file_mapping, FILE_MAP_READ, 0, 0, 0);
	p_file = m_file;

	//设置基本信息变量，并移动文件指针
	SetPublicHeaderBlock();
	SetPrivateHeaderBlock();
	SetDevicesInfoBlock();
	SetFirstFrame();
	//SetLastFrame(); //直接合并到SetFrameCount()中，效率提高一倍
	SetFrameCount();
	SetSecondCount();
	InitializeFPP();

	//TEST
	/*cout << dec << "frame_count = " << frame_count << endl;
	cout << dec << "second_count = " << second_count << endl;
	llong index_0 = ExtractData(125, 0);
	llong index_0_0 = ExtractData(0, 0);
	cout << dec << frame.package[0].point_2[0].x << endl;
	cout << dec << index_0 << endl;
	llong index_1 = ExtractData(7, 1);
	llong index_1_1 = ExtractData(2, 1);
	llong index_1_2 = ExtractData(7, 2);
	cout << dec << index_1 << endl;
	cout << dec << data[data.size() - 2].point_2[0].x << endl;
	cout << dec << file_size << endl;*/
	//printf("second_count = %d\n", second_count);
	//DWORD s = GetTickCount();
	//for (int i=0; i<second_count; i++)
	//{
	//	//MoveToTime(i); //测试843兆的lvx文件的时间：92867ms
	//	//MoveToSecond(i); //测试843兆的lvx文件的时间：733ms
	//	//ExtractData(i, 1); //测试843兆的lvx文件的时间：极慢
	//	//ExtractData(i, 2); //测试843兆的lvx文件的时间：161289ms
	//	//ExtractData(i, 3); //测试843兆的lvx文件的时间：70840ms
	//}
	////for (int i = 0; i<frame_count; i++)
	////{
	////	MoveToFrame(i); //测试843兆的lvx文件的时间：0ms
	////}
	///*ExtractData(1, 0);
	//printf("point_count_in_frame = %d\n", point_count_in_frame);*/
	//DWORD e = GetTickCount();
	//printf("%d ms\n", e - s);
}

//析构函数
lvx_x::lvxx::~lvxx()
{
	CloseAllFileHandles();
	//释放vector类型成员变量的内存（针对调用了resize()的vector成员变量）
	vector<P_2AT>().swap(p_2at);
	vector<POINT_2>().swap(temp_p2at.point_2);
	vector<char *>().swap(p_package);
}

//获取文件字节数
__int64 lvx_x::lvxx::GetFileLength()
{
	return file_size;
}

//获取帧数目
llong lvx_x::lvxx::GetFrameCount()
{
	return frame_count;
}

//获取整秒数目
llong lvx_x::lvxx::GetSecondCount()
{
	return second_count;
}

//获取整个lvx文件中的点数目
llong lvx_x::lvxx::GetPointsCount()
{
	return points_count_in_lvx;
}

//获取PUBLIC_HEADER_BLOCK
lvx_x::PUBLIC_HEADER_BLOCK lvx_x::lvxx::GetPublicHeaderBlock()
{
	return public_header_block;
}

//获取PRIVATE_HEADER_BLOCK
lvx_x::PRIVATE_HEADER_BLOCK lvx_x::lvxx::GetPrivateHeaderBlock()
{
	return private_header_block;
}

//获取DEVICES_INFO_BLOCK
lvx_x::DEVICES_INFO_BLOCK lvx_x::lvxx::GetDevicesInfoBlock()
{
	return devices_info_block;
}

//设置PUBLIC_HEADER_BLOCK
void lvx_x::lvxx::SetPublicHeaderBlock()
{
	for (int i = 0; i<sizeof(public_header_block.file_signature); i++)
	{
		public_header_block.file_signature[i] = *p_file;
		p_file++;
	}

	public_header_block.version_a = *p_file;
	p_file += sizeof(public_header_block.version_a);

	public_header_block.version_b = *p_file;
	p_file += sizeof(public_header_block.version_b);

	public_header_block.version_c = *p_file;
	p_file += sizeof(public_header_block.version_c);

	public_header_block.version_d = *p_file;
	p_file += sizeof(public_header_block.version_d);

	//连续取sizeof(uint)字节，但p_file指针位置并没有自动变化，所以仍要移动p_file
	public_header_block.magic_code = *(uint *)p_file;
	p_file += sizeof(public_header_block.magic_code);
}

//获取PRIVATE_HEADER_BLOCK
void lvx_x::lvxx::SetPrivateHeaderBlock()
{
	private_header_block.frame_duration = *(uint *)p_file;
	p_file += sizeof(private_header_block.frame_duration);

	private_header_block.device_count = *(uchar *)p_file;
	p_file += sizeof(private_header_block.device_count);
}

//获取DEVICES_INFO_BLOCK
void lvx_x::lvxx::SetDevicesInfoBlock()
{
	for (int i=0; i<sizeof(devices_info_block.lidar_sn_code); i++)
	{
		devices_info_block.lidar_sn_code[i] = *p_file;
		p_file ++;
	}

	for (int i = 0; i<sizeof(devices_info_block.hub_sn_code); i++)
	{
		devices_info_block.hub_sn_code[i] = *p_file;
		p_file++;
	}

	devices_info_block.device_index = *(uchar *)p_file;
	p_file += sizeof(devices_info_block.device_index);

	devices_info_block.device_type = *(uchar *)p_file;
	p_file += sizeof(devices_info_block.device_type);

	devices_info_block.extrinsic_enable = *(uchar *)p_file;
	p_file += sizeof(devices_info_block.extrinsic_enable);

	devices_info_block.roll = *(float *)p_file;
	p_file += sizeof(devices_info_block.roll);

	devices_info_block.pitch = *(float *)p_file;
	p_file += sizeof(devices_info_block.pitch);

	devices_info_block.yaw = *(float *)p_file;
	p_file += sizeof(devices_info_block.yaw);

	devices_info_block.x = *(float *)p_file;
	p_file += sizeof(devices_info_block.x);

	devices_info_block.y = *(float *)p_file;
	p_file += sizeof(devices_info_block.y);

	devices_info_block.z = *(float *)p_file;
	p_file += sizeof(devices_info_block.z);
}

//设置文件总帧数目（顺便初始化最后一帧的首指针）
void lvx_x::lvxx::SetFrameCount()
{
	p_file = first_frame;

	while (p_file - m_file + 1 <= file_size)
	{
		//初始化最后一帧的首指针
		last_frame = p_file;

		p_file += sizeof(llong);
		p_file += sizeof(llong);

		frame_count = *(llong *)p_file;

		p_file -= sizeof(llong);
		p_file = m_file + *(llong *)p_file;
	}

	frame_count++;
}

//第一帧首地址
void lvx_x::lvxx::SetFirstFrame()
{
	first_frame = m_file + PUBLIC_HEADER_BLOCK_SIZE
		+ PRIVATE_HEADER_BLOCK_SIZE + DEVICES_INFO_BLOCK_SIZE;
}

//最后一帧首地址
//void lvx_x::lvxx::SetLastFrame()
//{
//	p_file = first_frame;
//
//	while (true)
//	{
//		p_file += sizeof(llong);
//
//		if (*(llong *)p_file < file_size)
//		{
//			p_file = m_file + *(llong *)p_file;
//		}
//		else
//		{
//			//最后一帧的首地址
//			p_file -= sizeof(llong);
//			break;
//		}
//	}
//
//	last_frame = p_file;
//}

//设置整秒数目（顺便初始化isfpps）
void lvx_x::lvxx::SetSecondCount()
{
	//lvx文件中的整秒数目
	second_count = 0;
	//lvx文件中一共有多少个点
	points_count_in_lvx = 0;
	//将文件指针移动到lvx文件第一帧首指针
	p_file = first_frame;
	//isfpps
	ISFPP isfpp;
	//第0秒的开始帧与开始包地址（这里还得判断datatype是否为2）
	//因为第一帧第一个包的datatype不一定就是2
	for (llong i = 0; i < frame_count; i++)
	{
		bool mark = false;

		//当前帧（即第i帧）的首地址
		char *currentFrame = p_file;

		//下一帧首地址
		char *next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//第i帧第1个包的首地址
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataType为2
			if (*(uchar *)p_file == 2)
			{
				isfpp.beginFrame = currentFrame;
				isfpp.beginPackage = p_file - 10;
				mark = true;
				break;
			}
			//DataType为6
			else
			{
				p_file += 9;
				p_file += POINT_6_SIZE;
			}
		}
		if (mark)
		{
			break;
		}
	}

	p_file = first_frame;
	char *next_frame;
	llong timestamp = 0;
	char *currentFrame;

	for (llong i = 0; i<frame_count; i++)
	{
		//当前帧（即第i帧）的首地址
		currentFrame = p_file;

		//下一帧首地址
		next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//第i帧第1个包的首地址
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataType为2
			if (*(uchar *)p_file == 2)
			{
				if (timestamp <= *(llong *)(p_file + 1))
				{
					timestamp = *(llong *)(p_file + 1);
					//当前帧（即第i帧）的首地址，也是第second_count索引秒的结束帧指针
					isfpp.endFrame = currentFrame;
					//当前包的首地址
					isfpp.endPackage = p_file - 10;
				}
				else
				{
					isfpp.secondIndex = second_count;
					isfpps.push_back(isfpp);

					second_count++;

					isfpp.beginFrame = currentFrame;
					isfpp.beginPackage = p_file - 10;

					timestamp = *(llong *)(p_file + 1);
				}

				p_file += 9;
				p_file += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;

				points_count_in_lvx += 96;
			}
			//DataType为6
			else
			{
				p_file += 9;
				p_file += POINT_6_SIZE;
			}
		}
	}

	isfpp.secondIndex = second_count;
	isfpps.push_back(isfpp);

	second_count++;
}

//将文件指针移动到指定索引帧首地址并返回指针值
char *lvx_x::lvxx::MoveToFrame(llong index)
{
	//首帧首地址
	if (index <= 0)
	{
		p_file = first_frame;
	}
	//尾帧首地址
	else if (index >= frame_count - 1)
	{
		p_file = last_frame;
	}
	//寻找首帧与尾帧之间的帧的首地址
	else
	{
		//将文件移到指定的帧首地址
		p_file = CompareWithFPP(index);
		while (true)
		{
			p_file += sizeof(llong);
			p_file += sizeof(llong);

			if (*(llong *)p_file == index)
			{
				p_file -= sizeof(llong);
				p_file -= sizeof(llong);
				break;
			}
			else
			{
				p_file -= sizeof(llong);
				p_file = m_file + *(llong *)p_file;
			}
		}

		//更新当前地址
		fpp.current = p_file;
		fpp.currentIndex = index;
	}

	return p_file;
}

//将文件指针移动到指定索引整秒内每个包中首点的首地址（此函数从1.0.0.7版本开始不再推荐使用）
//推荐使用MoveToSecond函数
vector<char *> lvx_x::lvxx::MoveToTime(llong index)
{
	vector<char *> p_point;

	llong current_second_count = 0;
	p_file = first_frame;

	char *next_frame;
	llong timestamp = 0;

	for (llong i = 0; i<frame_count; i++)
	{
		//下一帧首地址
		next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//第i帧第1个包的首地址
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataType为2
			if (*(uchar *)p_file == 2)
			{
				if (timestamp <= *(llong *)(p_file + 1))
				{
					timestamp = *(llong *)(p_file + 1);
				}
				else
				{
					current_second_count++;

					if (index < 0 && current_second_count == 1)
					{
						return p_point;
					}
					//显然，current_second_count最多只能到second_count - 1
					else if (index == current_second_count - 1)
					{
						return p_point;
					}

					timestamp = *(llong *)(p_file + 1);

					p_point.clear();
					vector<char *>().swap(p_point);
				}

				p_file += 9;

				p_point.push_back(p_file);

				p_file += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;
			}
			//DataType为6
			else
			{
				p_file += 9;
				p_file += POINT_6_SIZE;
			}
		}
	}

	//index大于或等于second_count - 1时
	return p_point;
}

//将文件指针移动到指定索引整秒内每个包的首地址并保存（注意：这里只取datatype为2的包）
vector<char *> lvx_x::lvxx::MoveToSecond(llong index)
{
	//存放index索引整秒内每个datatype为2的包的首地址
	//清空vector数组，但数组内存容量仍然存在
	p_package.clear();

	char *beginFrame;
	char *endFrame;
	char *endPackage;
	//第一个包的datatype一定为2，因为在SetSecondCount函数中已经确定好了
	char *pFile;
	if (index < 0)
	{
		beginFrame = isfpps[0].beginFrame;
		endFrame = isfpps[0].endFrame;
		endPackage = isfpps[0].endPackage;
		pFile = isfpps[0].beginPackage;
	}
	else if (index > isfpps.size() - 1)
	{
		beginFrame = isfpps[isfpps.size() - 1].beginFrame;
		endFrame = isfpps[isfpps.size() - 1].endFrame;
		endPackage = isfpps[isfpps.size() - 1].endPackage;
		pFile = isfpps[isfpps.size() - 1].beginPackage;
	}
	else
	{
		beginFrame = isfpps[index].beginFrame;
		endFrame = isfpps[index].endFrame;
		endPackage = isfpps[index].endPackage;
		pFile = isfpps[index].beginPackage;
	}
	
	//下一帧首地址
	char *next_frame = m_file + *(llong *)(beginFrame + sizeof(llong));

	while (true)
	{
		while (pFile < next_frame)
		{
			pFile += 10;
			//DataType为2
			if (*(uchar *)pFile == 2)
			{
				p_package.push_back(pFile - 10);

				pFile += 9;
				pFile += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;
			}
			//DataType为6
			else
			{
				pFile += 9;
				pFile += POINT_6_SIZE;
			}
		}

		next_frame = m_file + *(llong *)(pFile + sizeof(llong));
		pFile += FRAME_HEADER_SIZE;

		//整秒最后一帧
		if (pFile - FRAME_HEADER_SIZE == endFrame)
		{
			while (pFile <= endPackage)
			{
				pFile += 10;
				//DataType为2
				if (*(uchar *)pFile == 2)
				{
					p_package.push_back(pFile - 10);

					pFile += 9;
					pFile += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;
				}
				//DataType为6
				else
				{
					pFile += 9;
					pFile += POINT_6_SIZE;
				}
			}

			break;
		}
	}

	return p_package;
}

//按帧/秒索引提取数据：0-帧索引；1-时间索引方式1；2-时间索引方式2；3-时间索引方式3
llong lvx_x::lvxx::ExtractData(llong index, const int mode)
{
	//按帧索引
	if (mode == FRAME_INDEX)
	{
		point_count_in_frame = 0;
		ClearFramePackage();

		MoveToFrame(index);

		//下一帧首地址
		char *next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		frame.frame_header.current_offset = *(llong *)p_file;
		p_file += sizeof(frame.frame_header.current_offset);

		frame.frame_header.next_offset = *(llong *)p_file;
		p_file += sizeof(frame.frame_header.next_offset);

		frame.frame_header.frame_index = *(llong *)p_file;
		p_file += sizeof(frame.frame_header.frame_index);

		while (p_file < next_frame)
		{
			package.device_index = *(uchar *)p_file;
			p_file += sizeof(package.device_index);

			package.version = *(uchar *)p_file;
			p_file += sizeof(package.version);

			package.slot_id = *(uchar *)p_file;
			p_file += sizeof(package.slot_id);

			package.lidar_id = *(uchar *)p_file;
			p_file += sizeof(package.lidar_id);

			package.reserved = *(uchar *)p_file;
			p_file += sizeof(package.reserved);

			package.status_code = *(uint *)p_file;
			p_file += sizeof(package.status_code);

			package.timestamp_type = *(uchar *)p_file;
			p_file += sizeof(package.timestamp_type);

			package.data_type = *(uchar *)p_file;
			p_file += sizeof(package.data_type);

			package.timestamp = *(llong *)p_file;
			p_file += sizeof(package.timestamp);

			//这里一定要对package.data_type进行判断，否则会引起重大错误
			if (package.data_type == 6)
			{
				p_file += POINT_6_SIZE;
			}
			else //package.data_type必定为2（采集数据的时候可以设置激光雷达数据类型）
			{
				for (int i = 0; i<POINT_NUM_IN_EACH_PACKAGE; i++)
				{
					point_2.x = *(int *)p_file;
					p_file += sizeof(point_2.x);

					point_2.y = *(int *)p_file;
					p_file += sizeof(point_2.y);

					point_2.z = *(int *)p_file;
					p_file += sizeof(point_2.z);

					point_2.reflectivity = *(uchar *)p_file;
					p_file += sizeof(point_2.reflectivity);

					point_2.tag = *(uchar *)p_file;
					p_file += sizeof(point_2.tag);

					package.point_2.push_back(point_2);
				}
				frame.package.push_back(package);
				point_count_in_frame += POINT_NUM_IN_EACH_PACKAGE;

				//需要清空vector<type>变量，这包括清空元素和内存
				package.point_2.clear(); //元素个数size()就变为0
				vector<POINT_2>().swap(package.point_2); //内存大小capacity()就变为0
			}
		}

		if (index < 0)
		{
			return 0;
		}
		else if (index > frame_count - 1)
		{
			return frame_count - 1;
		}
		else
		{
			return index;
		}
	}
	//按秒索引-方式1（不推荐）
	else if (mode == TIME_INDEX_1)
	{
		point_count_in_data = 0;
		data.clear();
		vector<PACKAGE>().swap(data);

		//根据索引参数求出帧索引（首尾）和包索引（首尾）
		llong temp_second_count = 0;
		p_file = first_frame;

		char *next_frame;
		llong timestamp = 0;

		for (llong i = 0; i<frame_count; i++)
		{
			//下一帧首地址
			next_frame = m_file + *(llong *)(p_file + sizeof(llong));

			//第i帧第1个包的首地址
			p_file += FRAME_HEADER_SIZE;

			while (p_file < next_frame)
			{
				package.device_index = *(uchar *)p_file;
				p_file += sizeof(package.device_index);

				package.version = *(uchar *)p_file;
				p_file += sizeof(package.version);

				package.slot_id = *(uchar *)p_file;
				p_file += sizeof(package.slot_id);

				package.lidar_id = *(uchar *)p_file;
				p_file += sizeof(package.lidar_id);

				package.reserved = *(uchar *)p_file;
				p_file += sizeof(package.reserved);

				package.status_code = *(uint *)p_file;
				p_file += sizeof(package.status_code);

				package.timestamp_type = *(uchar *)p_file;
				p_file += sizeof(package.timestamp_type);

				package.data_type = *(uchar *)p_file;
				p_file += sizeof(package.data_type);

				package.timestamp = *(llong *)p_file;
				p_file += sizeof(package.timestamp);

				//DataType为2
				if (package.data_type == 2)
				{
					if (timestamp <= package.timestamp)
					{
						timestamp = package.timestamp;
					}
					else
					{
						temp_second_count++;
						timestamp = package.timestamp;

						if (index <= 0 && temp_second_count == 1)
						{
							return i;
						}
						else if (temp_second_count - 1 == index)
						{
							return i;
						}

						point_count_in_data = 0;
						data.clear();
						vector<PACKAGE>().swap(data);
					}

					for (int j=0; j<POINT_NUM_IN_EACH_PACKAGE; j++)
					{
						point_2.x = *(int *)p_file;
						p_file += sizeof(point_2.x);

						point_2.y = *(int *)p_file;
						p_file += sizeof(point_2.y);

						point_2.z = *(int *)p_file;
						p_file += sizeof(point_2.z);

						point_2.reflectivity = *(uchar *)p_file;
						p_file += sizeof(point_2.reflectivity);

						point_2.tag = *(uchar *)p_file;
						p_file += sizeof(point_2.tag);

						package.point_2.push_back(point_2);
					}
					data.push_back(package);
					point_count_in_data += POINT_NUM_IN_EACH_PACKAGE;

					package.point_2.clear();
					vector<POINT_2>().swap(package.point_2);
				}
				//DataType为6
				else
				{
					p_file += POINT_6_SIZE;
				}
			}
		}
	
		return frame_count - 1;
	}
	//按秒索引-方式2（比方式1更优，但仍不推荐）
	else if (mode == TIME_INDEX_2)
	{
		point_count = 0;
		point.clear();
		vector<POINT_2>().swap(point);

		vector<char *> p_point = MoveToTime(index);

		POINT_2 temp_point;
		char *temp_p_file;

		//TEST
		//printf("p_point.size() = %d\n", p_point.size());

		for (int i=0; i<p_point.size(); i++)
		{
			temp_p_file = p_point[i];
			for (int j=0; j<POINT_NUM_IN_EACH_PACKAGE; j++)
			{
				temp_point.x = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.x);

				temp_point.y = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.y);

				temp_point.z = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.z);

				temp_point.reflectivity = *(uchar *)(temp_p_file);
				temp_p_file += sizeof(temp_point.reflectivity);

				temp_point.tag = *(uchar *)(temp_p_file);
				temp_p_file += sizeof(temp_point.tag);

				point_count++;
				point.push_back(temp_point);
			}
		}

		return point_count;
	}
	//按秒索引-方式3（比方式2更优，推荐使用）
	else if (mode == TIME_INDEX_3)
	{
		pointsCount = 0;
		p_2at.clear();

		MoveToSecond(index);
		char *temp_p_file;

		//TEST
		//printf("p_package.size() = %d\n", p_package.size());

		POINT_2 temp_point;

		for (int i = 0; i<p_package.size(); i++)
		{
			//第i个包的首地址
			temp_p_file = p_package[i];
			temp_p_file += 11;
			temp_p2at.timestamp = *(llong *)temp_p_file;
			temp_p_file += 8;
			for (int j = 0; j<POINT_NUM_IN_EACH_PACKAGE; j++)
			{
				temp_point.x = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.x);

				temp_point.y = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.y);

				temp_point.z = *(int *)(temp_p_file);
				temp_p_file += sizeof(temp_point.z);

				temp_point.reflectivity = *(uchar *)(temp_p_file);
				temp_p_file += sizeof(temp_point.reflectivity);

				temp_point.tag = *(uchar *)(temp_p_file);
				temp_p_file += sizeof(temp_point.tag);

				temp_p2at.point_2.push_back(temp_point);
			}

			pointsCount += POINT_NUM_IN_EACH_PACKAGE;
			p_2at.push_back(temp_p2at);
			temp_p2at.point_2.clear();
		}

		return pointsCount;
	}
	//非法mode
	else
	{
		return -1;
	}
}

//初始化帧指针结构体变量
void lvx_x::lvxx::InitializeFPP()
{
	fpp.begin = first_frame;
	fpp.beginIndex = 0;

	fpp.end = last_frame;
	fpp.endIndex = frame_count - 1;

	fpp.current = first_frame;
	fpp.currentIndex = 0;
	
	llong i = 0;
	p_file = first_frame;
	p_file += sizeof(llong);
	p_file += sizeof(llong);
	while (*(llong *)p_file < frame_count / 2)
	{
		//计算中间帧指针
		fpp.middle = p_file - 2 * sizeof(llong);
		fpp.middleIndex = *(llong *)p_file;

		p_file -= sizeof(llong);
		p_file = m_file + *(llong *)p_file;

		p_file += sizeof(llong);
		p_file += sizeof(llong);
	}
}

//求绝对值
inline llong lvx_x::lvxx::Abs(const llong &a, const llong &b)
{
	return a - b > 0 ? a - b : b - a;
}

//将帧索引与各个预设位置比较并返回最近的位置指针
//index-帧索引
char *lvx_x::lvxx::CompareWithFPP(const llong &index)
{
	//先根据begin、middle、end分成2段，然后再根据current把每段分成
	if (index>fpp.beginIndex && index<fpp.middleIndex)
	{
		if (fpp.currentIndex>fpp.beginIndex && fpp.currentIndex<fpp.middleIndex)
		{
			if (index < fpp.currentIndex)
			{
				p_file = fpp.begin;
			}
			else
			{
				p_file = fpp.current;
			}
		}
		else
		{
			p_file = fpp.begin;
		}
	}
	else
	{
		if (fpp.currentIndex > fpp.middleIndex && fpp.currentIndex < fpp.endIndex)
		{
			if (index < fpp.currentIndex)
			{
				p_file = fpp.middle;
			}
			else
			{
				p_file = fpp.current;
			}
		}
		else
		{
			p_file = fpp.middle;
		}
	}

	return p_file;
}

//清空包
void lvx_x::lvxx::ClearFramePackage()
{
	frame.package.clear();
	vector<PACKAGE>().swap(frame.package);
}

//关闭所有文件句柄
void lvx_x::lvxx::CloseAllFileHandles()
{
	UnmapViewOfFile(m_file);
	CloseHandle(h_file_mapping);
	CloseHandle(h_file);
}
