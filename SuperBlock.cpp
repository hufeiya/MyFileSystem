#include "SuperBlock.h"



SuperBlock::SuperBlock()
{
}

/**
*	初始化超级快,仅在磁盘使用初次调用
*/
void SuperBlock::initSuperBlock()
{
	for (short i = 0; i < 200; i++)
	{
		blockStack[i] = i + 4;
	}
	freeBlockNum = 200;
	for (short i = 0; i < 40; i++)
	{
		inodeStack[i] = i;
	}
	inodeNum = 40;
	blockLock = false;
	iLock = false;
	modifySignal = false;
	time((time_t*)&modifyTime);
}

/**
*	打印超级块信息
*/
void SuperBlock::print()
{
	cout << BLOCK_NUM << endl;
	for (int i = 0; i < 200; i++)
	{
		cout << blockStack[i] << " ";
	}
	cout << endl;
	cout << freeBlockNum << endl;
	for (int i = 0; i < 40; i++)
	{
		cout << inodeStack[i] << " ";
	}
	cout << endl;
	cout << inodeNum << endl;
	cout << blockLock << endl;
	cout << iLock << endl;
	cout << modifySignal << endl;
	cout << modifyTime << endl;
}

/**	分配盘块号	*/
short SuperBlock::allocBlock()
{
	if (modifySignal)
	{
		cerr << "超级块正在使用中,请稍后再试" << endl;
		return NONE;
	}
	else if (freeBlockNum <= 0)
	{
		cerr << "暂无空闲盘块,请稍后再试" << endl;
		return NONE;
	}
	else
	{
		modifySignal = true;
		short blockNum = blockStack[--freeBlockNum];
		modifySignal = false;
		return blockNum;
	}

}

/**	回收盘块	*/
void SuperBlock::retrieveBlock(short blockNum)
{
	if (modifySignal)
	{
		cerr << "超级块正在使用中,请稍后再试" << endl;
	}
	else if (freeBlockNum >= BLOCK_NUM)
	{
		cerr << "盘块已全部回收,无须再回收" << endl;
	}
	else
	{
		modifySignal = true;
		blockStack[freeBlockNum++] = blockNum;
		modifySignal = false;
	}
}

/**	分配FCB	*/
short SuperBlock::allocFCB()
{
	if (modifySignal)
	{
		cerr << "超级块正在使用中,请稍后再试" << endl;
		return NONE;
	}
	else if (inodeNum <= 0)
	{
		cerr << "暂无空闲索引块,请稍后再试" << endl;
		return NONE;
	}
	else
	{

		modifySignal = true;
		short num = inodeStack[--inodeNum];
		modifySignal = false;
		return num;
	}

}

/**	回收FCB	*/
void SuperBlock::retrieveFCB(short fcbNum)
{
	if (modifySignal)
	{
		cerr << "超级块正在使用中,请稍后再试" << endl;
	}
	else if (inodeNum >= 40)
	{
		cerr << "索引块已全部回收,无须再回收" << endl;
	}
	else
	{
		modifySignal = true;
		inodeStack[inodeNum++] = fcbNum;
		modifySignal = false;
	}
}
SuperBlock::~SuperBlock()
{
}
