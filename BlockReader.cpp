#include "BlockReader.h"


/**
*	默认构造,使用默认磁盘DISK_NAME
*/
BlockReader::BlockReader()
{
	f = new fstream(DISK_NAME, ios_base::binary | ios_base::in | ios_base::out);
}

/**
*	自定义构造,使用自定义磁盘名
*/
BlockReader::BlockReader(char* diskName)
{
	f = new fstream(diskName, ios_base::binary | ios_base::in | ios_base::out);
}

/**
*读取指定盘块号的内容
*/
char * BlockReader::readBlock(int num)
{
	char * result = new char[BLOCK_SIZE];
	f->seekg(BLOCK_SIZE*num);
	f->read(result, BLOCK_SIZE);
	return result;
}

/**
*写入内容到指定盘块
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
