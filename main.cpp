#include <iostream>
#include <fstream>
#include "commonConstValue.h"
#include "BlockReader.h"
#include "SuperBlock.h"
#include "Shell.h"
#include "UserItem.h"
#include "MD5.h"
using namespace std;

/**填充模拟磁盘*/
void fillTheVirtualDisk()
{
	const char* diskName = DISK_NAME;
	fstream f(diskName, ios_base::binary | ios_base::out);
	const int blockNum = BLOCK_NUM;//盘块数目
	char *blank = new char[BLOCK_SIZE];
	for (int i = 0; i < 512; i++)
	{
			blank[i] = 0;
		
	}
	for (int i = 0; i < blockNum; i++) 
	{
		f.write(blank, 512);
	}
	f.close();
	delete[] blank;

}

/**测试盘块读取写入*/
void testReadAndWrite()
{
	BlockReader* blockReader = new BlockReader();
	char* fromDisk = NULL;
	char* toDisk = new char[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		toDisk[i] = i % 128;
	}
	blockReader->writeBlock(0, toDisk);
	fromDisk = blockReader->readBlock(0);
	for (int i = 0; i < 128; i++)
	{
		cout << fromDisk[i];
	}
	delete[] toDisk;
	delete[] fromDisk;
	delete blockReader;
	system("pause");
}

/**初始化超级块,仅在磁盘使用初次调用*/
void initSuperBlock()
{
	BlockReader* reader = new BlockReader();
	SuperBlock* s = new SuperBlock();
	s->initSuperBlock();
	//cout << sizeof(*s);
	//system("pause");
	reader->writeBlock(0,(char*)s);
	delete s;
	delete reader;
	
}

/**打印超级块信息*/
void printSuperBlockInfo()
{
	BlockReader* reader = new BlockReader();
	SuperBlock* s;
	s = (SuperBlock*)reader->readBlock(0);
	s->print();
	delete s;
	system("pause");
}
/**测试root目录写入*/
void testRootBlock()
{
	BlockReader* reader = new BlockReader();
	char* block = reader->readBlock(4);
	block[0] = '.';
	block[1] = '.';
	block[14] = 4;
	block[16] = '.';
	block[30] = 4;
	reader->writeBlock(4, block);
	delete block;
	delete reader;
	
}


int main()
{
	//fillTheVirtualDisk();
	//testReadAndWrite();
	//initSuperBlock();
	//printSuperBlockInfo();
	//testRootBlock();
	Shell* shell = new Shell();
	delete shell;
	
	return 0;
}