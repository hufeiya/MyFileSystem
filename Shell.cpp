#include "Shell.h"



/**从磁盘获取索引节点(FCB)*/
FCB * Shell::getInode(short inodeNum)
{
	if (inodeNum == NONE)
	{
		return nullptr;
	}
	if (memoryInodeMap.find(inodeNum) != memoryInodeMap.end())//在内存节点中找到直接返回
	{
		return memoryInodeMap[inodeNum]->fCB;
	}
	short blockNum = inodeNum / 16 + 1;	//所在盘块号
	short shifting = inodeNum % 16 * 32;//所在盘块偏移
	FCB* fCB = nullptr;
	char* block = nullptr;
	block = readBlock(blockNum);//读取盘块(从磁盘或缓存)
	fCB = (FCB*)(block + shifting);
	fCB->num = inodeNum;
	//生成内存索引节点
	MemoryInode* memoryInode = new MemoryInode();
	memoryInode->num = inodeNum;
	memoryInode->fCB = fCB;
	memoryInodeMap[inodeNum] = memoryInode;//保存内存索引节点到哈希表         
	return fCB;
}

/**保存索引节点到磁盘*/
void Shell::saveInode(FCB * fCB)
{
	short blockNum = fCB->num / 16 + 1;	//所在盘块号
	char* block = blockBuffer[blockNum];//从盘块缓存哈希表中读取盘块地址
	writeBlock(blockNum, block);
}

char* Shell::readBlock(short blockNum)
{
	if (blockNum == NONE)
	{
		return nullptr;
	}
	if (blockBuffer.find(blockNum) != blockBuffer.end())//找到缓存
	{
		return blockBuffer[blockNum];
	}
	if (superBlock == nullptr)//还没读入超级块
	{
		if (blockNum != 0)
		{
			cerr << "还没读入超级块,请先读入" << endl;
			return nullptr;
		}
		else
		{
			return blockReader->readBlock(0);
		}
	}
	else if (superBlock->modifySignal)
	{
		cerr << "未获取超级块锁,请稍后再试" << endl;
		return nullptr;
	}
	else if (superBlock->blockLock)
	{
		cerr << "未获取盘块锁,请稍后再试" << endl;
		return nullptr;
	}
	else
	{

		if (blockNum != 0)
		{
			//保存到内存缓存,不保存超级块,防止清除缓存时误清除超级块
			blockBuffer[blockNum] = blockReader->readBlock(blockNum);
		}
		
		return blockBuffer[blockNum];
	}
}

void Shell::writeBlock(short num,char * block)
{
	if (superBlock->modifySignal)
	{
		cerr << "未获取超级块锁,请稍后再试" << endl;
	}
	else if (superBlock->blockLock)
	{
		cerr << "未获取盘块锁,请稍后再试" << endl;
	}
	else
	{
		if (num != 0)
		{
			superBlock->blockLock = true;
		}
		blockReader->writeBlock(num,block);
		superBlock->blockLock = false;
	}
}

Shell::Shell()
{
	blockReader = new BlockReader();
	superBlock = (SuperBlock*)readBlock(0);
	//读取根节点目录,位置固定在4号盘块
	currentCatalog = (Catalog*)readBlock(4);
	currentFCBNum = ROOT_FCB_NUM;
	//获取输入,循环
	string input;
	cout << "$ ";
	while (getline(cin,input))
	{
		vector<string> v = split(input);
		if (v.empty())
		{
			continue;
		}
		string operation, param;
		operation = v[0];
		if (v.size() > 1)
		{
			param = v[1];
		}
		if (operation == "ls")
		{
			ls(param);
		}
		else if (operation == "mkdir")
		{
			mkdir(param);
		}
		else if (operation == "cd")
		{
			cd(param);
		}
		else if (operation == "rmdir")
		{
			rmdir(param);
		}
		else if (operation == "umask")
		{
			umask(param);
		}
		else if (operation == "cat")
		{
			cat(param);
		}
		else if (operation == "vi")
		{
			vi(param);
		}
		else if (operation == "rm")
		{
			rm(param);
		}
		else if (operation == "mv")
		{
			mv(v[1], v[2]);
		}
		else if (operation == "ln")
		{
			ln(v[1], v[2]);
		}
		else if (operation == "chmod")
		{
			chmod(v[1], v[2]);
		}
		else if (operation == "cp")
		{
			cp(v[1], v[2]);
		}
		else if (operation == "chown")
		{
			chown(atoi(v[1].c_str()), v[2]);
		}
		else if (operation == "chgrp")
		{
			chgrp(v[1][0], v[2]);
		}
		else if (operation == "pwd")
		{
			pwdInfo();
		}
		else if (operation == "useradd")
		{
			useradd(param);
		}
		else if (operation == "printSuperBlock")
		{
			superBlock->print();
		}
		else if (operation == "printFCB")
		{
			getInode(currentFCBNum)->print();
		}
		else if (operation == "help")
		{
			help();
		}
		else if (operation == "find")
		{
			findFile(param);
		}
		else if (operation == "exit")
		{
			break;
		}


		cout << endl;
		cout << "$ ";
	}
	
}

void Shell::mkdir(string dirName)
{
	FCB* newFCB = mkFileInode(dirName);
	if (newFCB == nullptr)
	{
		cerr << "没有空闲索引节点,新建失败";
		return;
	}
	newFCB->type = 'd';
	//分配盘块
	short blockNum = superBlock->allocBlock();
	if(blockNum == NONE)//盘块已满
	{
		return;
	}
	newFCB->address[0] = blockNum;
	saveInode(newFCB);//写回FCB
	Catalog* catalog = new Catalog();
	memset(catalog, 0, 512);//初始化全0
	//定义父目录
	strcpy_s(catalog->items[0].fileName, "..");
	catalog->items[0].inode = currentFCBNum;
	//定义当前目录
	strcpy_s(catalog->items[1].fileName, ".");
	catalog->items[1].inode = newFCB->num;
	//写回当前盘块
	writeCurrentBlock();
	//写回目录盘块
	writeBlock(blockNum, (char*)catalog);
	//写回超级块
	writeBlock(0, (char*)superBlock);

	delete catalog;
}

void Shell::ls(string param)
{
	if (param == "-l")
	{
		//TODO
	}
	else
	{
		currentCatalog->print();
	}
}

void Shell::cd(string dirName)
{
	if (dirName == "/")
	{
		currentCatalog = (Catalog*)readBlock(4);
		currentFCBNum = ROOT_FCB_NUM;
		return;
	}
	if (dirName.back() == '/')
	{
		dirName = dirName.substr(0, dirName.length() - 1);
	}
	string nextDir;
	int slash = dirName.find('/');
	if (slash != string::npos)
	{
		nextDir = dirName.substr(slash + 1);
		dirName = dirName.substr(0, slash);
		if (slash == 0)
		{
			pwd.clear();
			dirName = "/";
			currentCatalog = (Catalog*)readBlock(4);
			currentFCBNum = ROOT_FCB_NUM;
			cd(nextDir);
			return;
		}
	}
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << dirName << " 这个目录.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, dirName.c_str()) == 0)//找到
		{
			//如果是返回root目录
			if (currentCatalog->items[i].inode == ROOT_FCB_NUM)
			{
				pwd.clear();
				currentFCBNum = ROOT_FCB_NUM;
				currentCatalog = (Catalog*)readBlock(4);
				if ( ! nextDir.empty())
				{
					cd(nextDir);//递归
				}
				return;
			}
			FCB* targetInode = ((FCB*)getInode(currentCatalog->items[i].inode));
			if (targetInode->type != 'd')//不是目录就继续查找
			{
				continue;
			}
			if (dirName == "..")
			{
				pwd.pop_back();
			}
			else
			{
				pwd.push_back(dirName);
			}
			currentFCBNum = targetInode->num;
			currentCatalog = (Catalog*)readBlock(targetInode->address[0]);
			if ( ! nextDir.empty())
			{
				cd(nextDir);//递归
			}
			return;
		}
	}
}
void Shell::cat(string path)
{
	FCB* fcb = findFCBFromPath(path);
	if (fcb == nullptr)
	{
		cerr << "没有找到文件 " << path;
		return;
	}
	cout << read(fcb, 0, fcb->length);

}
void Shell::rmdir(string dirName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cerr << "没有 " << dirName << " 这个目录.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, dirName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			if (fcb->type != 'd')//如果不是目录,继续查找
			{
				continue;
			}
			
			Catalog* catalog = (Catalog*)readBlock(fcb->address[0]);
			if (catalog->items[2].inode != 0 && fcb->accessCount == 0)
			{
				cerr << "目录不为空,无法删除" ;
				return;
			}
			if (fcb->accessCount != 0)
			{
				fcb->accessCount--;
				saveInode(fcb);
			}
			else
			{
				superBlock->retrieveBlock(fcb->address[0]);
				delete catalog;//释放盘块内存
				blockBuffer.erase(fcb->address[0]);//清除盘块缓存
				MemoryInode* minode = memoryInodeMap[fcb->num];
				delete minode;//释放内存索引
				memoryInodeMap.erase(fcb->num);//清除内存索引缓存
				superBlock->retrieveFCB(fcb->num);
				memset(fcb, 0, 32);//归零磁盘索引
			}
			memset(&currentCatalog->items[i], 0, 16);//清除目录项
			for (int j = i+1; j < 32; j++)//后面目录项往前挪
			{
				if (currentCatalog->items[j].inode == 0)
				{
					break;
				}
				currentCatalog->items[j - 1] = currentCatalog->items[j];
				currentCatalog->items[j].inode = 0;//判断空只判断iNode,所以只修改iNode为0就行了
			}
			writeCurrentBlock();
			return;
		}
	}
}

void Shell::vi(string path)
{
	system("cls");
	FCB* fcb = findFCBFromPath(path);
	if (fcb == nullptr)
	{
		cout << "[新文件]: " << endl;
		fcb = mkFileInode(path);
		if (fcb == nullptr)
		{
			cerr << "没有空闲索引节点,新建失败";
			return;
		}
	}
	else
	{
		cout << "原文件内容:" << endl;
		cat(path);
		cout << endl << "请输入新文件内容:" << endl;
	}
	string content,line;
	getline(cin, line);
	while (line != ":wq")
	{
		content += line + '\n';
		getline(cin, line);
	}
	if (write(fcb, content))
	{
		cout << "写入成功!" << endl;
	}
	else
	{
		cout << "写入失败,磁盘已满" << endl;
	}
	fcb->mask();
	fcb->type = '-';
	fcb->length = content.size();
	saveInode(fcb);

}

void Shell::rm(string fileName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cerr << "没有 " << fileName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			if (fcb->type == 'd')//如果是目录,继续查找
			{
				continue;
			}
			if (fcb->accessCount != 0)
			{
				fcb->accessCount--;
				saveInode(fcb);
				return;
			}
			//释放0次间址
			for (int i = 0; i < 4; i++)
			{
				if (fcb->address[i] != 0)
				{
					superBlock->retrieveBlock(fcb->address[i]);
					unordered_map<short, char*>::const_iterator got = blockBuffer.find(fcb->address[i]);
					if (got != blockBuffer.end())
					{
						delete[] blockBuffer[fcb->address[i]];
						blockBuffer.erase(fcb->address[i]);//清除盘块缓存
					}
					
					fcb->address[i] = 0;
				}
			}
			//释放一次间址
			if (fcb->address[4] != 0)
			{
				short* index = (short*)readBlock(fcb->address[4]);
				for (int i = 0; i < BLOCK_SIZE / 2; i++)
				{
					if (index[i] == 0)
					{
						break;
					}
					superBlock->retrieveBlock(index[i]);
					unordered_map<short, char*>::const_iterator got = blockBuffer.find(index[i]);
					if (got != blockBuffer.end())
					{
						delete[] blockBuffer[index[i]];
						blockBuffer.erase(index[i]);//清除盘块缓存
					}
					index[i] = 0;
				}
				superBlock->retrieveBlock(fcb->address[4]);
				unordered_map<short, char*>::const_iterator got = blockBuffer.find(fcb->address[4]);
				if (got != blockBuffer.end())
				{
					delete[] blockBuffer[fcb->address[4]];
					blockBuffer.erase(fcb->address[4]);//清除盘块缓存
				}
				fcb->address[4] = 0;
			}
			MemoryInode* minode = memoryInodeMap[fcb->num];
			delete minode;//释放内存索引
			memoryInodeMap.erase(fcb->num);//清除内存索引缓存
			superBlock->retrieveFCB(fcb->num);
			memset(fcb, 0, 32);//归零磁盘索引
			memset(&currentCatalog->items[i], 0, 16);//清除目录项
			for (int j = i + 1; j < 32; j++)//后面目录项往前挪
			{
				if (currentCatalog->items[j].inode == 0)
				{
					break;
				}
				currentCatalog->items[j - 1] = currentCatalog->items[j];
				currentCatalog->items[j].inode = 0;//判断空只判断iNode,所以只修改iNode为0就行了
			}
			writeCurrentBlock();
			return;
		}
	}
}

void Shell::useradd(string username)
{
	User* user = (User*)readBlock(6);
	for (int i = 0; i < 8; i++)
	{
		if (strcmp(user->users[i].name, username.c_str()) == 0)
		{
			cout << username << " 用户已经存在";
			break;
		}
		if (user->users[i].name[0] == 0)
		{
			cout << "请输入密码:" << endl;
			string pass;
			cin >> pass;
			MD5 iMD5;
			iMD5.GenerateMD5((unsigned char*)pass.c_str(), strlen(pass.c_str()));
			string encryptedPass = iMD5.ToString();
			user->users[i].id = i;
			user->users[i].groupNum = 0;
			strcpy(user->users[i].home,username.c_str());
			strcpy(user->users[i].name, username.c_str());
			strcpy(user->users[i].password, encryptedPass.c_str());
			//保存当前目录,因为cd会改变它
			Catalog* catalogBak = currentCatalog;
			int fcbNumBak = currentFCBNum;
			//回到root目录
			currentCatalog = (Catalog*)readBlock(4);
			currentFCBNum = ROOT_FCB_NUM;
			//在home下新建用户家目录
			cd("home");
			mkdir(username);
			//还原当前目录
			currentCatalog = catalogBak;
			currentFCBNum = fcbNumBak;
			break;
		}
	}
	writeBlock(198, (char*)user);
}

void Shell::mv(string oldName, string newName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << oldName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, oldName.c_str()) == 0)//找到
		{
			strcpy(currentCatalog->items[i].fileName, newName.c_str());
			if (currentFCBNum == ROOT_FCB_NUM)
			{
				writeBlock(4, (char*)currentCatalog);
			}
			else
			{
				writeBlock(getInode(currentFCBNum)->address[0], (char*)currentCatalog);
			}
			break;
			
		}
	}
}




/**字符串分割函数,用于读取命令和参数*/
vector<string> Shell::split(string str)
{
	string pattern = " ";
	string::size_type pos;
	vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

/**写回当前盘块*/
void Shell::writeCurrentBlock()
{
	
	short currentBlockNum = 4;
	if (currentFCBNum != ROOT_FCB_NUM)
	{
		currentBlockNum = getInode(currentFCBNum)->address[0];
	}
	writeBlock(currentBlockNum, (char*)currentCatalog);
}

/**私有函数,给文件分配节点,不分配盘块*/
FCB * Shell::mkFileInode(string fileName)
{
	CatalogItem *item = nullptr;
	//查找当前目录的空目录项
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			item = &currentCatalog->items[i];
			break;
		}
	}
	if (item == nullptr)
	{
		cerr << "目录已满,不能新建" << endl;
		return nullptr;
	}
	//分配索引节点
	FCB* newFCB = getInode(superBlock->allocFCB());
	if (newFCB == nullptr)//没有分配成功
	{
		return nullptr;
	}
	if (fileName.length() > 13) {//文件名超过13字节的删去
		fileName = fileName.substr(0, 13);
	}
	strcpy_s(item->fileName, fileName.c_str());
	item->inode = newFCB->num;
	newFCB->mask();
	return newFCB;
}

FCB * Shell::findFCBFromPath(string path)
{
	//纯文件名,不包含路径
	if (path.find('/') == string::npos)
	{
		return findFCBFromFileNameInCurrentDir(path);
	}
	int slashPos = path.find_last_of('/');
	string fileName = path.substr(slashPos + 1);
	path = path.substr(0, slashPos);
	//保存当前目录,因为cd会改变它
	Catalog* catalogBak = currentCatalog;
	int fcbNumBak = currentFCBNum;
	cd(path);
	FCB* fcb = findFCBFromFileNameInCurrentDir(fileName);
	//还原当前目录
	currentCatalog = catalogBak;
	currentFCBNum = fcbNumBak;
	return fcb;
}

FCB * Shell::findFCBFromFileNameInCurrentDir(string fileName)
{
	CatalogItem *item = nullptr;
	for (int i = 0; i < 32; i++)
	{
		if (strcmp(currentCatalog->items[i].fileName,fileName.c_str()) == 0)
		{
			item = &currentCatalog->items[i];
			break;
		}
	}
	if (item == nullptr)//没有找到文件
	{
		return nullptr;
	}
	FCB* fcb = getInode(item->inode);
	return fcb;
}

string Shell::read(FCB * fcb, short start, short end)
{
	string content = "";
	vector<char*>blocks;
	int blockNum = fcb->length / BLOCK_SIZE;
	int shift = fcb->length % BLOCK_SIZE;
	if (shift == 0)
	{
		blockNum++;
	}
	short startNum, startShift, endNum, endShift;
	startNum = start / BLOCK_SIZE;
	startShift = start % BLOCK_SIZE;
	endNum = end / BLOCK_SIZE;
	endShift = end % (BLOCK_SIZE+1);//注意此处+1
	if (endShift % BLOCK_SIZE == 0)
	{
		endNum--;
	}
	if (endNum < 4)//使用直接块号
	{
		for (int i = startNum; i <= endNum; i++)
		{
			blocks.push_back(readBlock(fcb->address[i]));
		}
	}
	//使用直接块号和一次间址
	else if (startNum < 4 && endNum > 4 && endNum < 4 + BLOCK_SIZE / 2)
	{
		for (int i = startNum; i < 4; i++)
		{
			blocks.push_back(readBlock(fcb->address[i]));
		}
		short* blockIndex = (short*)readBlock(fcb->address[4]);
		for (int i = 0; i < endNum - 4; i++)
		{
			blocks.push_back(readBlock(blockIndex[i-4]));
		}
	}
	//使用一次间址
	else if (startNum > 4 && endNum > 4 && endNum < 4 + BLOCK_SIZE / 2)
	{
		short* blockIndex = (short*)readBlock(fcb->address[4]);
		for (int i = startNum - 4; i < endNum - 4; i++)
		{
			blocks.push_back(readBlock(blockIndex[i - 4]));
		}
	}
	//生成string
	for (int i = 0; i < blocks.size(); i++)
	{
		string temp = string(blocks[i],BLOCK_SIZE);
		if (i == 0)
		{
			
			temp= temp.substr(startShift);
		}
		if (i == blocks.size() - 1)
		{
			temp = temp.substr(0, endShift - startShift);
		}
		content += temp;
	}
	return content;
}

bool Shell::write(FCB * fcb, string content)
{
	vector<string> v;
	//分割字符串成每个512字节
	for (int i = 0; i < content.size(); i+=BLOCK_SIZE)
	{
		v.push_back(content.substr(i, (content.size() - i < BLOCK_SIZE ? content.size()-i:BLOCK_SIZE)));
	}
	if (v.size() <= 4)//使用直接块号
	{
		for (int i = 0; i < v.size(); i++)
		{
			if (fcb->address[i] == 0)
			{
				fcb->address[i] = superBlock->allocBlock();
			}
			writeBlock(fcb->address[i], (char*)v[i].c_str());
		}
	}
	//使用直接块号和一次间址
	else if (v.size() < 4 + BLOCK_SIZE / 2)
	{
		for (int i = 0; i < 4; i++)
		{
			if (fcb->address[i] == 0)
			{
				fcb->address[i] = superBlock->allocBlock();
			}
			writeBlock(fcb->address[i], (char*)v[i].c_str());
		}
		if (fcb->address[4] == 0)
		{
			fcb->address[4] = superBlock->allocBlock();
		}
		short* blockIndex = new short[BLOCK_SIZE / 2];
		for (int i = 4; i < v.size(); i++)
		{
			blockIndex[i - 4] = superBlock->allocBlock();
			writeBlock(blockIndex[i - 4], (char*)v[i].c_str());
		}
		writeBlock(fcb->address[4], (char*)blockIndex);
		delete[] blockIndex;
	}

	return true;
}

void Shell::chmod(string newMod, string fileName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << fileName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			const char* mod = newMod.c_str();
			for (int i = 0; i < 3; i++)
			{
				fcb->authority[i] = mod[i];
			}
			saveInode(fcb);
			break;
		}
	}
}

void Shell::chown(short ownId, string fileName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << fileName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			fcb->owner = ownId;
			saveInode(fcb);
			break;
		}
	}
}

void Shell::chgrp(char groupId, string fileName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << fileName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			fcb->group = groupId;
			saveInode(fcb);
			break;
		}
	}
}

void Shell::ln(string newfile, string exitedFile)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << exitedFile << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, exitedFile.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			//找空白目录项
			for (int j = i;j < 32; j++)
			{
				if (currentCatalog->items[j].inode == 0)
				{
					strcpy(currentCatalog->items[j].fileName, newfile.c_str());
					currentCatalog->items[j].inode = fcb->num;
					fcb->accessCount++;
					saveInode(fcb);
					if (currentFCBNum == ROOT_FCB_NUM)
					{
						writeBlock(4, (char*)currentCatalog);
					}
					else
					{
						writeBlock(getInode(currentFCBNum)->address[0], (char*)currentCatalog);
					}
					return;
				}
			}
			cerr << "没有空目录项";
			return;
		}
	}
}

void Shell::pwdInfo()
{
	if (pwd.size() == 0)
	{
		cout << "/";
	}
	for (int i = 0; i < pwd.size(); i++)
	{
		cout << "/" << pwd[i];
	}
}

void Shell::umask(string fileName)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << fileName << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//找到
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			fcb->mask();
			saveInode(fcb);
			break;
		}
	}
}

void Shell::cp(string from, string dest)
{
	for (int i = 0; i < 32; i++)
	{
		if (currentCatalog->items[i].inode == 0)
		{
			cout << "没有 " << from << " 这个文件.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, from.c_str()) == 0)//找到
		{
			
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			FCB* newFCB = mkFileInode(dest);
			if (newFCB == nullptr)
			{
				cerr << "没有空闲索引节点,新建失败";
				return;
			}
			newFCB->type = fcb->type;
			newFCB->mask();
			newFCB->length = fcb->length;
			write(newFCB, read(fcb, 0, fcb->length));
			saveInode(newFCB);
			writeCurrentBlock();
			break;
			
		}
	}
}

void Shell::findFile( string filename)
{

	for (int i = 0; i < 200; i++)
	{
		Catalog* catalog = (Catalog*)readBlock(i);
		for (int j = 0; j < 32; j++)
		{
			if (strcmp(currentCatalog->items[j].fileName, filename.c_str()) == 0)//找到
			{
				cout << "找到";
				return;
			}
		}

	}
}

void Shell::help()
{
	cout << "Ls				显示文件目录" << endl;	cout << "Chmod			改变文件权限" << endl;	cout << "Chown			改变文件拥有者" << endl;	cout << "Chgrp			改变文件所属组" << endl;	cout << "Pwd			显示当前目录" << endl;	cout << "Cd				改变当前目录" << endl;	cout << "Mkdir			创建子目录" << endl;	cout << "Rmdir			删除子目录" << endl;	cout << "Umask			文件创建屏蔽码" << endl;	cout << "Mv				改变文件名" << endl;	cout << "Cp				文件拷贝" << endl;	cout << "Rm				文件删除" << endl;	cout << "Ln           	建立文件联接" << endl;	cout << "Cat			连接显示文件内容" << endl;	cout << "Passwd			修改用户口令" << endl;	cout << "Help			显示所有命令的帮助文档； 输入某个命令 + ？时显示" << endl;	cout << "				该条命令的使用说明" << endl;	cout << "Exit           退出系统" << endl;
}

Shell::~Shell()
{
	//退出前释放所有锁,保存超级块,回收内存
	superBlock->modifySignal = false;
	superBlock->blockLock = false;
	superBlock->iLock = false;
	writeCurrentBlock();
	blockReader->writeBlock(0, (char*)superBlock);
	delete superBlock;
	delete blockReader;
}
