#include "lasx.h"

lasx_x::lasx::lasx(const char *fPath)
{
	//����CString��ǿ������ֲ�ԣ�ͬʱ��Ӧchar��wchar_t���ͣ�
	CString filePath(fPath);

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

	//TEET
	//std::cout << std::dec << file_size << std::endl;
	//std::cout << *(ushort *)(p_file + 4) << std::endl;
	printf("%d\n", *(ushort *)(p_file + 4));
}


lasx_x::lasx::~lasx()
{

}

