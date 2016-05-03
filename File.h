#pragma once
#include "FCB.h"
#include <string>
using namespace std;
class File
{
public:
	string path;
	FCB *fcb;
	File();
	File(string path);
	string read(short start, short end);
	void write(short start, string content);
	~File();
};

