#pragma once
#include <fstream>
#include "commonConstValue.h"
using namespace std;
class BlockReader
{
private:
	fstream* f = NULL;
public:
	BlockReader();
	BlockReader(char *diskName);
	char* readBlock(int num);
	void writeBlock(int num,char* content);
	
	~BlockReader();
};

