#pragma once
#include <string>
#include <vector>
#include <stack>
#include "SuperBlock.h"
#include "MemoryInode.h"
#include "FCB.h"
#include "BlockReader.h"
#include "Catalog.h"
#include <unordered_map>
#include "commonConstValue.h"
#include "User.h"
#include "UserItem.h"
#include "MD5.h"
using namespace std;
class Shell
{
private:
	BlockReader* blockReader;							//盘块读写类
	SuperBlock* superBlock;								//超级盘块
	Catalog* currentCatalog;							//当前目录
	unordered_map<short, MemoryInode*> memoryInodeMap;	//内存索引节点
	unordered_map<short, char*> blockBuffer;			//盘块在内存的缓存
	short currentFCBNum;								//当前目录的FCB号
	vector<string> pwd;									//当前目录
	
	FCB* getInode(short inodeNum);
	void saveInode(FCB * fCB);
	char* readBlock(short blockNum);
	void writeBlock(short num,char* block);
	vector<string> split(string str);
	void writeCurrentBlock();
	FCB* mkFileInode(string fileName);
	FCB* findFCBFromPath(string path);
	FCB* findFCBFromFileNameInCurrentDir(string fileName);
	string read(FCB* fcb, short start, short end);
	bool write(FCB* fcb, string content);
	
public:
	Shell();
	void mkdir(string dirName);
	void ls(string param);
	void cd(string dirName);
	void cat(string path);
	void rmdir(string dirName);
	void vi(string path);
	void rm(string fileName);
	void useradd(string username);
	void mv(string oldName, string newName);
	void chmod(string newMod, string fileName);
	void chown(short ownId, string fileName);
	void chgrp(char groupId, string fileName);
	void ln(string newfile, string exitedFile);
	void pwdInfo();
	void umask(string fileName);
	void cp(string from, string dest);
	void findFile(string filename);
	void help();
	
	~Shell();
};

