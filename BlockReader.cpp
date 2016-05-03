#include "BlockReader.h"


/**
*	Ĭ�Ϲ���,ʹ��Ĭ�ϴ���DISK_NAME
*/
BlockReader::BlockReader()
{
	f = new fstream(DISK_NAME, ios_base::binary | ios_base::in | ios_base::out);
}

/**
*	�Զ��幹��,ʹ���Զ��������
*/
BlockReader::BlockReader(char* diskName)
{
	f = new fstream(diskName, ios_base::binary | ios_base::in | ios_base::out);
}

/**
*��ȡָ���̿�ŵ�����
*/
char * BlockReader::readBlock(int num)
{
	char * result = new char[BLOCK_SIZE];
	f->seekg(BLOCK_SIZE*num);
	f->read(result, BLOCK_SIZE);
	return result;
}

/**
*д�����ݵ�ָ���̿�
*/
void BlockReader::writeBlock(int num,char * content)
{
	f->seekg(BLOCK_SIZE*num);
	f->write(content, BLOCK_SIZE);
}



BlockReader::~BlockReader()
{
	if (f != NULL)
	{
		f->close();
		delete f;
	}
}
