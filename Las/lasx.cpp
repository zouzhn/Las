#include "lasx.h"

lasx_x::lasx::lasx(const char *fPath)
{
	//利用CString增强程序移植性（同时适应char和wchar_t类型）
	CString filePath(fPath);

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

	//TEET
	//std::cout << std::dec << file_size << std::endl;
	//std::cout << *(ushort *)(p_file + 4) << std::endl;
	printf("%d\n", *(ushort *)(p_file + 4));
}


lasx_x::lasx::~lasx()
{

}

