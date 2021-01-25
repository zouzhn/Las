#include "lvxx-1.0.0.7.h"

//���캯��
lvx_x::lvxx::lvxx(const char *pfile)
{
	//����CString��ǿ������ֲ�ԣ�ͬʱ��Ӧchar��wchar_t���ͣ�
	CString filePath(pfile);

	//������ӳ����ļ���ӳ����������ˮ���ϵĵ�Ӱ����Ȼ���ü�����������ʵ���ڵģ�
	h_file = CreateFile(filePath, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//�õ������ļ��Ĵ�С
	DWORD hi; //��32λ
	file_size = GetFileSize(h_file, &hi); //��32λ
	file_size |= (((__int64)hi) << 32); //�ļ���С

	//�����ļ�ӳ����
	h_file_mapping = CreateFileMapping(h_file, NULL, PAGE_READONLY, NULL, NULL, NULL);

	//ӳ�������ļ�
	m_file = (char *)MapViewOfFile(h_file_mapping, FILE_MAP_READ, 0, 0, 0);
	p_file = m_file;

	//���û�����Ϣ���������ƶ��ļ�ָ��
	SetPublicHeaderBlock();
	SetPrivateHeaderBlock();
	SetDevicesInfoBlock();
	SetFirstFrame();
	//SetLastFrame(); //ֱ�Ӻϲ���SetFrameCount()�У�Ч�����һ��
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
	//	//MoveToTime(i); //����843�׵�lvx�ļ���ʱ�䣺92867ms
	//	//MoveToSecond(i); //����843�׵�lvx�ļ���ʱ�䣺733ms
	//	//ExtractData(i, 1); //����843�׵�lvx�ļ���ʱ�䣺����
	//	//ExtractData(i, 2); //����843�׵�lvx�ļ���ʱ�䣺161289ms
	//	//ExtractData(i, 3); //����843�׵�lvx�ļ���ʱ�䣺70840ms
	//}
	////for (int i = 0; i<frame_count; i++)
	////{
	////	MoveToFrame(i); //����843�׵�lvx�ļ���ʱ�䣺0ms
	////}
	///*ExtractData(1, 0);
	//printf("point_count_in_frame = %d\n", point_count_in_frame);*/
	//DWORD e = GetTickCount();
	//printf("%d ms\n", e - s);
}

//��������
lvx_x::lvxx::~lvxx()
{
	CloseAllFileHandles();
	//�ͷ�vector���ͳ�Ա�������ڴ棨��Ե�����resize()��vector��Ա������
	vector<P_2AT>().swap(p_2at);
	vector<POINT_2>().swap(temp_p2at.point_2);
	vector<char *>().swap(p_package);
}

//��ȡ�ļ��ֽ���
__int64 lvx_x::lvxx::GetFileLength()
{
	return file_size;
}

//��ȡ֡��Ŀ
llong lvx_x::lvxx::GetFrameCount()
{
	return frame_count;
}

//��ȡ������Ŀ
llong lvx_x::lvxx::GetSecondCount()
{
	return second_count;
}

//��ȡ����lvx�ļ��еĵ���Ŀ
llong lvx_x::lvxx::GetPointsCount()
{
	return points_count_in_lvx;
}

//��ȡPUBLIC_HEADER_BLOCK
lvx_x::PUBLIC_HEADER_BLOCK lvx_x::lvxx::GetPublicHeaderBlock()
{
	return public_header_block;
}

//��ȡPRIVATE_HEADER_BLOCK
lvx_x::PRIVATE_HEADER_BLOCK lvx_x::lvxx::GetPrivateHeaderBlock()
{
	return private_header_block;
}

//��ȡDEVICES_INFO_BLOCK
lvx_x::DEVICES_INFO_BLOCK lvx_x::lvxx::GetDevicesInfoBlock()
{
	return devices_info_block;
}

//����PUBLIC_HEADER_BLOCK
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

	//����ȡsizeof(uint)�ֽڣ���p_fileָ��λ�ò�û���Զ��仯��������Ҫ�ƶ�p_file
	public_header_block.magic_code = *(uint *)p_file;
	p_file += sizeof(public_header_block.magic_code);
}

//��ȡPRIVATE_HEADER_BLOCK
void lvx_x::lvxx::SetPrivateHeaderBlock()
{
	private_header_block.frame_duration = *(uint *)p_file;
	p_file += sizeof(private_header_block.frame_duration);

	private_header_block.device_count = *(uchar *)p_file;
	p_file += sizeof(private_header_block.device_count);
}

//��ȡDEVICES_INFO_BLOCK
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

//�����ļ���֡��Ŀ��˳���ʼ�����һ֡����ָ�룩
void lvx_x::lvxx::SetFrameCount()
{
	p_file = first_frame;

	while (p_file - m_file + 1 <= file_size)
	{
		//��ʼ�����һ֡����ָ��
		last_frame = p_file;

		p_file += sizeof(llong);
		p_file += sizeof(llong);

		frame_count = *(llong *)p_file;

		p_file -= sizeof(llong);
		p_file = m_file + *(llong *)p_file;
	}

	frame_count++;
}

//��һ֡�׵�ַ
void lvx_x::lvxx::SetFirstFrame()
{
	first_frame = m_file + PUBLIC_HEADER_BLOCK_SIZE
		+ PRIVATE_HEADER_BLOCK_SIZE + DEVICES_INFO_BLOCK_SIZE;
}

//���һ֡�׵�ַ
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
//			//���һ֡���׵�ַ
//			p_file -= sizeof(llong);
//			break;
//		}
//	}
//
//	last_frame = p_file;
//}

//����������Ŀ��˳���ʼ��isfpps��
void lvx_x::lvxx::SetSecondCount()
{
	//lvx�ļ��е�������Ŀ
	second_count = 0;
	//lvx�ļ���һ���ж��ٸ���
	points_count_in_lvx = 0;
	//���ļ�ָ���ƶ���lvx�ļ���һ֡��ָ��
	p_file = first_frame;
	//isfpps
	ISFPP isfpp;
	//��0��Ŀ�ʼ֡�뿪ʼ����ַ�����ﻹ���ж�datatype�Ƿ�Ϊ2��
	//��Ϊ��һ֡��һ������datatype��һ������2
	for (llong i = 0; i < frame_count; i++)
	{
		bool mark = false;

		//��ǰ֡������i֡�����׵�ַ
		char *currentFrame = p_file;

		//��һ֡�׵�ַ
		char *next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//��i֡��1�������׵�ַ
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataTypeΪ2
			if (*(uchar *)p_file == 2)
			{
				isfpp.beginFrame = currentFrame;
				isfpp.beginPackage = p_file - 10;
				mark = true;
				break;
			}
			//DataTypeΪ6
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
		//��ǰ֡������i֡�����׵�ַ
		currentFrame = p_file;

		//��һ֡�׵�ַ
		next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//��i֡��1�������׵�ַ
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataTypeΪ2
			if (*(uchar *)p_file == 2)
			{
				if (timestamp <= *(llong *)(p_file + 1))
				{
					timestamp = *(llong *)(p_file + 1);
					//��ǰ֡������i֡�����׵�ַ��Ҳ�ǵ�second_count������Ľ���ָ֡��
					isfpp.endFrame = currentFrame;
					//��ǰ�����׵�ַ
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
			//DataTypeΪ6
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

//���ļ�ָ���ƶ���ָ������֡�׵�ַ������ָ��ֵ
char *lvx_x::lvxx::MoveToFrame(llong index)
{
	//��֡�׵�ַ
	if (index <= 0)
	{
		p_file = first_frame;
	}
	//β֡�׵�ַ
	else if (index >= frame_count - 1)
	{
		p_file = last_frame;
	}
	//Ѱ����֡��β֮֡���֡���׵�ַ
	else
	{
		//���ļ��Ƶ�ָ����֡�׵�ַ
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

		//���µ�ǰ��ַ
		fpp.current = p_file;
		fpp.currentIndex = index;
	}

	return p_file;
}

//���ļ�ָ���ƶ���ָ������������ÿ�������׵���׵�ַ���˺�����1.0.0.7�汾��ʼ�����Ƽ�ʹ�ã�
//�Ƽ�ʹ��MoveToSecond����
vector<char *> lvx_x::lvxx::MoveToTime(llong index)
{
	vector<char *> p_point;

	llong current_second_count = 0;
	p_file = first_frame;

	char *next_frame;
	llong timestamp = 0;

	for (llong i = 0; i<frame_count; i++)
	{
		//��һ֡�׵�ַ
		next_frame = m_file + *(llong *)(p_file + sizeof(llong));

		//��i֡��1�������׵�ַ
		p_file += FRAME_HEADER_SIZE;

		while (p_file < next_frame)
		{
			p_file += 10;

			//DataTypeΪ2
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
					//��Ȼ��current_second_count���ֻ�ܵ�second_count - 1
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
			//DataTypeΪ6
			else
			{
				p_file += 9;
				p_file += POINT_6_SIZE;
			}
		}
	}

	//index���ڻ����second_count - 1ʱ
	return p_point;
}

//���ļ�ָ���ƶ���ָ������������ÿ�������׵�ַ�����棨ע�⣺����ֻȡdatatypeΪ2�İ���
vector<char *> lvx_x::lvxx::MoveToSecond(llong index)
{
	//���index����������ÿ��datatypeΪ2�İ����׵�ַ
	//���vector���飬�������ڴ�������Ȼ����
	p_package.clear();

	char *beginFrame;
	char *endFrame;
	char *endPackage;
	//��һ������datatypeһ��Ϊ2����Ϊ��SetSecondCount�������Ѿ�ȷ������
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
	
	//��һ֡�׵�ַ
	char *next_frame = m_file + *(llong *)(beginFrame + sizeof(llong));

	while (true)
	{
		while (pFile < next_frame)
		{
			pFile += 10;
			//DataTypeΪ2
			if (*(uchar *)pFile == 2)
			{
				p_package.push_back(pFile - 10);

				pFile += 9;
				pFile += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;
			}
			//DataTypeΪ6
			else
			{
				pFile += 9;
				pFile += POINT_6_SIZE;
			}
		}

		next_frame = m_file + *(llong *)(pFile + sizeof(llong));
		pFile += FRAME_HEADER_SIZE;

		//�������һ֡
		if (pFile - FRAME_HEADER_SIZE == endFrame)
		{
			while (pFile <= endPackage)
			{
				pFile += 10;
				//DataTypeΪ2
				if (*(uchar *)pFile == 2)
				{
					p_package.push_back(pFile - 10);

					pFile += 9;
					pFile += POINT_NUM_IN_EACH_PACKAGE * POINT_2_SIZE;
				}
				//DataTypeΪ6
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

//��֡/��������ȡ���ݣ�0-֡������1-ʱ��������ʽ1��2-ʱ��������ʽ2��3-ʱ��������ʽ3
llong lvx_x::lvxx::ExtractData(llong index, const int mode)
{
	//��֡����
	if (mode == FRAME_INDEX)
	{
		point_count_in_frame = 0;
		ClearFramePackage();

		MoveToFrame(index);

		//��һ֡�׵�ַ
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

			//����һ��Ҫ��package.data_type�����жϣ�����������ش����
			if (package.data_type == 6)
			{
				p_file += POINT_6_SIZE;
			}
			else //package.data_type�ض�Ϊ2���ɼ����ݵ�ʱ��������ü����״��������ͣ�
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

				//��Ҫ���vector<type>��������������Ԫ�غ��ڴ�
				package.point_2.clear(); //Ԫ�ظ���size()�ͱ�Ϊ0
				vector<POINT_2>().swap(package.point_2); //�ڴ��Сcapacity()�ͱ�Ϊ0
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
	//��������-��ʽ1�����Ƽ���
	else if (mode == TIME_INDEX_1)
	{
		point_count_in_data = 0;
		data.clear();
		vector<PACKAGE>().swap(data);

		//���������������֡��������β���Ͱ���������β��
		llong temp_second_count = 0;
		p_file = first_frame;

		char *next_frame;
		llong timestamp = 0;

		for (llong i = 0; i<frame_count; i++)
		{
			//��һ֡�׵�ַ
			next_frame = m_file + *(llong *)(p_file + sizeof(llong));

			//��i֡��1�������׵�ַ
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

				//DataTypeΪ2
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
				//DataTypeΪ6
				else
				{
					p_file += POINT_6_SIZE;
				}
			}
		}
	
		return frame_count - 1;
	}
	//��������-��ʽ2���ȷ�ʽ1���ţ����Բ��Ƽ���
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
	//��������-��ʽ3���ȷ�ʽ2���ţ��Ƽ�ʹ�ã�
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
			//��i�������׵�ַ
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
	//�Ƿ�mode
	else
	{
		return -1;
	}
}

//��ʼ��ָ֡��ṹ�����
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
		//�����м�ָ֡��
		fpp.middle = p_file - 2 * sizeof(llong);
		fpp.middleIndex = *(llong *)p_file;

		p_file -= sizeof(llong);
		p_file = m_file + *(llong *)p_file;

		p_file += sizeof(llong);
		p_file += sizeof(llong);
	}
}

//�����ֵ
inline llong lvx_x::lvxx::Abs(const llong &a, const llong &b)
{
	return a - b > 0 ? a - b : b - a;
}

//��֡���������Ԥ��λ�ñȽϲ����������λ��ָ��
//index-֡����
char *lvx_x::lvxx::CompareWithFPP(const llong &index)
{
	//�ȸ���begin��middle��end�ֳ�2�Σ�Ȼ���ٸ���current��ÿ�ηֳ�
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

//��հ�
void lvx_x::lvxx::ClearFramePackage()
{
	frame.package.clear();
	vector<PACKAGE>().swap(frame.package);
}

//�ر������ļ����
void lvx_x::lvxx::CloseAllFileHandles()
{
	UnmapViewOfFile(m_file);
	CloseHandle(h_file_mapping);
	CloseHandle(h_file);
}
