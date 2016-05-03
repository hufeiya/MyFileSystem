#pragma once
#include <iostream>
#include <time.h>
#include "FCB.h"
#include "commonConstValue.h"
using namespace std;
class SuperBlock
{
public:
	const short BLOCK_NUM = 200;
	short blockStack[200];
	short freeBlockNum;
	short inodeStack[40];
	short inodeNum;
	bool blockLock;
	bool iLock;
	bool modifySignal;
	int modifyTime;
	char notUse[16];
public:
	SuperBlock();
	void initSuperBlock();
	void print();
	short allocBlock();
	void retrieveBlock(short blockNum);
	short allocFCB();
	void retrieveFCB(short fcbNum);
	~SuperBlock();
};

