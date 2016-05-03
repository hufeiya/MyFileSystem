#pragma once
#include <time.h>
#include <iostream>
using namespace std;
class FCB
{
public:
	int length;
	int modifyTime;
	short num;
	short address[6];
	short accessCount;
	short owner;
	char group;
	char linkNum;
	char type;
	char authority[3];
	
public:
	FCB();
	void mask();
	void print();
	~FCB();
};

