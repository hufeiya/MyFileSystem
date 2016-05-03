#pragma once
#include "FCB.h"
class MemoryInode
{
public:
	int num;
	bool state;
	int count;
	int deviceNum;
	FCB* fCB;

public:
	MemoryInode();
	~MemoryInode();
};

