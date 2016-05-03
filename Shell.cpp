#include "Shell.h"



/**�Ӵ��̻�ȡ�����ڵ�(FCB)*/
FCB * Shell::getInode(short inodeNum)
{
	if (inodeNum == NONE)
	{
		return nullptr;
	}
	if (memoryInodeMap.find(inodeNum) != memoryInodeMap.end())//���ڴ�ڵ����ҵ�ֱ�ӷ���
	{
		return memoryInodeMap[inodeNum]->fCB;
	}
	short blockNum = inodeNum / 16 + 1;	//�����̿��
	short shifting = inodeNum % 16 * 32;//�����̿�ƫ��
	FCB* fCB = nullptr;
	char* block = nullptr;
	block = readBlock(blockNum);//��ȡ�̿�(�Ӵ��̻򻺴�)
	fCB = (FCB*)(block + shifting);
	fCB->num = inodeNum;
	//�����ڴ������ڵ�
	MemoryInode* memoryInode = new MemoryInode();
	memoryInode->num = inodeNum;
	memoryInode->fCB = fCB;
	memoryInodeMap[inodeNum] = memoryInode;//�����ڴ������ڵ㵽��ϣ��         
	return fCB;
}

/**���������ڵ㵽����*/
void Shell::saveInode(FCB * fCB)
{
	short blockNum = fCB->num / 16 + 1;	//�����̿��
	char* block = blockBuffer[blockNum];//���̿黺���ϣ���ж�ȡ�̿��ַ
	writeBlock(blockNum, block);
}

char* Shell::readBlock(short blockNum)
{
	if (blockNum == NONE)
	{
		return nullptr;
	}
	if (blockBuffer.find(blockNum) != blockBuffer.end())//�ҵ�����
	{
		return blockBuffer[blockNum];
	}
	if (superBlock == nullptr)//��û���볬����
	{
		if (blockNum != 0)
		{
			cerr << "��û���볬����,���ȶ���" << endl;
			return nullptr;
		}
		else
		{
			return blockReader->readBlock(0);
		}
	}
	else if (superBlock->modifySignal)
	{
		cerr << "δ��ȡ��������,���Ժ�����" << endl;
		return nullptr;
	}
	else if (superBlock->blockLock)
	{
		cerr << "δ��ȡ�̿���,���Ժ�����" << endl;
		return nullptr;
	}
	else
	{

		if (blockNum != 0)
		{
			//���浽�ڴ滺��,�����泬����,��ֹ�������ʱ�����������
			blockBuffer[blockNum] = blockReader->readBlock(blockNum);
		}
		
		return blockBuffer[blockNum];
	}
}

void Shell::writeBlock(short num,char * block)
{
	if (superBlock->modifySignal)
	{
		cerr << "δ��ȡ��������,���Ժ�����" << endl;
	}
	else if (superBlock->blockLock)
	{
		cerr << "δ��ȡ�̿���,���Ժ�����" << endl;
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
	//��ȡ���ڵ�Ŀ¼,λ�ù̶���4���̿�
	currentCatalog = (Catalog*)readBlock(4);
	currentFCBNum = ROOT_FCB_NUM;
	//��ȡ����,ѭ��
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
		cerr << "û�п��������ڵ�,�½�ʧ��";
		return;
	}
	newFCB->type = 'd';
	//�����̿�
	short blockNum = superBlock->allocBlock();
	if(blockNum == NONE)//�̿�����
	{
		return;
	}
	newFCB->address[0] = blockNum;
	saveInode(newFCB);//д��FCB
	Catalog* catalog = new Catalog();
	memset(catalog, 0, 512);//��ʼ��ȫ0
	//���常Ŀ¼
	strcpy_s(catalog->items[0].fileName, "..");
	catalog->items[0].inode = currentFCBNum;
	//���嵱ǰĿ¼
	strcpy_s(catalog->items[1].fileName, ".");
	catalog->items[1].inode = newFCB->num;
	//д�ص�ǰ�̿�
	writeCurrentBlock();
	//д��Ŀ¼�̿�
	writeBlock(blockNum, (char*)catalog);
	//д�س�����
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
			cout << "û�� " << dirName << " ���Ŀ¼.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, dirName.c_str()) == 0)//�ҵ�
		{
			//����Ƿ���rootĿ¼
			if (currentCatalog->items[i].inode == ROOT_FCB_NUM)
			{
				pwd.clear();
				currentFCBNum = ROOT_FCB_NUM;
				currentCatalog = (Catalog*)readBlock(4);
				if ( ! nextDir.empty())
				{
					cd(nextDir);//�ݹ�
				}
				return;
			}
			FCB* targetInode = ((FCB*)getInode(currentCatalog->items[i].inode));
			if (targetInode->type != 'd')//����Ŀ¼�ͼ�������
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
				cd(nextDir);//�ݹ�
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
		cerr << "û���ҵ��ļ� " << path;
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
			cerr << "û�� " << dirName << " ���Ŀ¼.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, dirName.c_str()) == 0)//�ҵ�
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			if (fcb->type != 'd')//�������Ŀ¼,��������
			{
				continue;
			}
			
			Catalog* catalog = (Catalog*)readBlock(fcb->address[0]);
			if (catalog->items[2].inode != 0 && fcb->accessCount == 0)
			{
				cerr << "Ŀ¼��Ϊ��,�޷�ɾ��" ;
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
				delete catalog;//�ͷ��̿��ڴ�
				blockBuffer.erase(fcb->address[0]);//����̿黺��
				MemoryInode* minode = memoryInodeMap[fcb->num];
				delete minode;//�ͷ��ڴ�����
				memoryInodeMap.erase(fcb->num);//����ڴ���������
				superBlock->retrieveFCB(fcb->num);
				memset(fcb, 0, 32);//�����������
			}
			memset(&currentCatalog->items[i], 0, 16);//���Ŀ¼��
			for (int j = i+1; j < 32; j++)//����Ŀ¼����ǰŲ
			{
				if (currentCatalog->items[j].inode == 0)
				{
					break;
				}
				currentCatalog->items[j - 1] = currentCatalog->items[j];
				currentCatalog->items[j].inode = 0;//�жϿ�ֻ�ж�iNode,����ֻ�޸�iNodeΪ0������
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
		cout << "[���ļ�]: " << endl;
		fcb = mkFileInode(path);
		if (fcb == nullptr)
		{
			cerr << "û�п��������ڵ�,�½�ʧ��";
			return;
		}
	}
	else
	{
		cout << "ԭ�ļ�����:" << endl;
		cat(path);
		cout << endl << "���������ļ�����:" << endl;
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
		cout << "д��ɹ�!" << endl;
	}
	else
	{
		cout << "д��ʧ��,��������" << endl;
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
			cerr << "û�� " << fileName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//�ҵ�
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			if (fcb->type == 'd')//�����Ŀ¼,��������
			{
				continue;
			}
			if (fcb->accessCount != 0)
			{
				fcb->accessCount--;
				saveInode(fcb);
				return;
			}
			//�ͷ�0�μ�ַ
			for (int i = 0; i < 4; i++)
			{
				if (fcb->address[i] != 0)
				{
					superBlock->retrieveBlock(fcb->address[i]);
					unordered_map<short, char*>::const_iterator got = blockBuffer.find(fcb->address[i]);
					if (got != blockBuffer.end())
					{
						delete[] blockBuffer[fcb->address[i]];
						blockBuffer.erase(fcb->address[i]);//����̿黺��
					}
					
					fcb->address[i] = 0;
				}
			}
			//�ͷ�һ�μ�ַ
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
						blockBuffer.erase(index[i]);//����̿黺��
					}
					index[i] = 0;
				}
				superBlock->retrieveBlock(fcb->address[4]);
				unordered_map<short, char*>::const_iterator got = blockBuffer.find(fcb->address[4]);
				if (got != blockBuffer.end())
				{
					delete[] blockBuffer[fcb->address[4]];
					blockBuffer.erase(fcb->address[4]);//����̿黺��
				}
				fcb->address[4] = 0;
			}
			MemoryInode* minode = memoryInodeMap[fcb->num];
			delete minode;//�ͷ��ڴ�����
			memoryInodeMap.erase(fcb->num);//����ڴ���������
			superBlock->retrieveFCB(fcb->num);
			memset(fcb, 0, 32);//�����������
			memset(&currentCatalog->items[i], 0, 16);//���Ŀ¼��
			for (int j = i + 1; j < 32; j++)//����Ŀ¼����ǰŲ
			{
				if (currentCatalog->items[j].inode == 0)
				{
					break;
				}
				currentCatalog->items[j - 1] = currentCatalog->items[j];
				currentCatalog->items[j].inode = 0;//�жϿ�ֻ�ж�iNode,����ֻ�޸�iNodeΪ0������
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
			cout << username << " �û��Ѿ�����";
			break;
		}
		if (user->users[i].name[0] == 0)
		{
			cout << "����������:" << endl;
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
			//���浱ǰĿ¼,��Ϊcd��ı���
			Catalog* catalogBak = currentCatalog;
			int fcbNumBak = currentFCBNum;
			//�ص�rootĿ¼
			currentCatalog = (Catalog*)readBlock(4);
			currentFCBNum = ROOT_FCB_NUM;
			//��home���½��û���Ŀ¼
			cd("home");
			mkdir(username);
			//��ԭ��ǰĿ¼
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
			cout << "û�� " << oldName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, oldName.c_str()) == 0)//�ҵ�
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




/**�ַ����ָ��,���ڶ�ȡ����Ͳ���*/
vector<string> Shell::split(string str)
{
	string pattern = " ";
	string::size_type pos;
	vector<std::string> result;
	str += pattern;//��չ�ַ����Է������
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

/**д�ص�ǰ�̿�*/
void Shell::writeCurrentBlock()
{
	
	short currentBlockNum = 4;
	if (currentFCBNum != ROOT_FCB_NUM)
	{
		currentBlockNum = getInode(currentFCBNum)->address[0];
	}
	writeBlock(currentBlockNum, (char*)currentCatalog);
}

/**˽�к���,���ļ�����ڵ�,�������̿�*/
FCB * Shell::mkFileInode(string fileName)
{
	CatalogItem *item = nullptr;
	//���ҵ�ǰĿ¼�Ŀ�Ŀ¼��
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
		cerr << "Ŀ¼����,�����½�" << endl;
		return nullptr;
	}
	//���������ڵ�
	FCB* newFCB = getInode(superBlock->allocFCB());
	if (newFCB == nullptr)//û�з���ɹ�
	{
		return nullptr;
	}
	if (fileName.length() > 13) {//�ļ�������13�ֽڵ�ɾȥ
		fileName = fileName.substr(0, 13);
	}
	strcpy_s(item->fileName, fileName.c_str());
	item->inode = newFCB->num;
	newFCB->mask();
	return newFCB;
}

FCB * Shell::findFCBFromPath(string path)
{
	//���ļ���,������·��
	if (path.find('/') == string::npos)
	{
		return findFCBFromFileNameInCurrentDir(path);
	}
	int slashPos = path.find_last_of('/');
	string fileName = path.substr(slashPos + 1);
	path = path.substr(0, slashPos);
	//���浱ǰĿ¼,��Ϊcd��ı���
	Catalog* catalogBak = currentCatalog;
	int fcbNumBak = currentFCBNum;
	cd(path);
	FCB* fcb = findFCBFromFileNameInCurrentDir(fileName);
	//��ԭ��ǰĿ¼
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
	if (item == nullptr)//û���ҵ��ļ�
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
	endShift = end % (BLOCK_SIZE+1);//ע��˴�+1
	if (endShift % BLOCK_SIZE == 0)
	{
		endNum--;
	}
	if (endNum < 4)//ʹ��ֱ�ӿ��
	{
		for (int i = startNum; i <= endNum; i++)
		{
			blocks.push_back(readBlock(fcb->address[i]));
		}
	}
	//ʹ��ֱ�ӿ�ź�һ�μ�ַ
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
	//ʹ��һ�μ�ַ
	else if (startNum > 4 && endNum > 4 && endNum < 4 + BLOCK_SIZE / 2)
	{
		short* blockIndex = (short*)readBlock(fcb->address[4]);
		for (int i = startNum - 4; i < endNum - 4; i++)
		{
			blocks.push_back(readBlock(blockIndex[i - 4]));
		}
	}
	//����string
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
	//�ָ��ַ�����ÿ��512�ֽ�
	for (int i = 0; i < content.size(); i+=BLOCK_SIZE)
	{
		v.push_back(content.substr(i, (content.size() - i < BLOCK_SIZE ? content.size()-i:BLOCK_SIZE)));
	}
	if (v.size() <= 4)//ʹ��ֱ�ӿ��
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
	//ʹ��ֱ�ӿ�ź�һ�μ�ַ
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
			cout << "û�� " << fileName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//�ҵ�
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
			cout << "û�� " << fileName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//�ҵ�
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
			cout << "û�� " << fileName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//�ҵ�
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
			cout << "û�� " << exitedFile << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, exitedFile.c_str()) == 0)//�ҵ�
		{
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			//�ҿհ�Ŀ¼��
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
			cerr << "û�п�Ŀ¼��";
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
			cout << "û�� " << fileName << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, fileName.c_str()) == 0)//�ҵ�
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
			cout << "û�� " << from << " ����ļ�.";
			return;
		}
		if (strcmp(currentCatalog->items[i].fileName, from.c_str()) == 0)//�ҵ�
		{
			
			FCB* fcb = getInode(currentCatalog->items[i].inode);
			FCB* newFCB = mkFileInode(dest);
			if (newFCB == nullptr)
			{
				cerr << "û�п��������ڵ�,�½�ʧ��";
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
			if (strcmp(currentCatalog->items[j].fileName, filename.c_str()) == 0)//�ҵ�
			{
				cout << "�ҵ�";
				return;
			}
		}

	}
}

void Shell::help()
{
	cout << "Ls				��ʾ�ļ�Ŀ¼" << endl;	cout << "Chmod			�ı��ļ�Ȩ��" << endl;	cout << "Chown			�ı��ļ�ӵ����" << endl;	cout << "Chgrp			�ı��ļ�������" << endl;	cout << "Pwd			��ʾ��ǰĿ¼" << endl;	cout << "Cd				�ı䵱ǰĿ¼" << endl;	cout << "Mkdir			������Ŀ¼" << endl;	cout << "Rmdir			ɾ����Ŀ¼" << endl;	cout << "Umask			�ļ�����������" << endl;	cout << "Mv				�ı��ļ���" << endl;	cout << "Cp				�ļ�����" << endl;	cout << "Rm				�ļ�ɾ��" << endl;	cout << "Ln           	�����ļ�����" << endl;	cout << "Cat			������ʾ�ļ�����" << endl;	cout << "Passwd			�޸��û�����" << endl;	cout << "Help			��ʾ��������İ����ĵ��� ����ĳ������ + ��ʱ��ʾ" << endl;	cout << "				���������ʹ��˵��" << endl;	cout << "Exit           �˳�ϵͳ" << endl;
}

Shell::~Shell()
{
	//�˳�ǰ�ͷ�������,���泬����,�����ڴ�
	superBlock->modifySignal = false;
	superBlock->blockLock = false;
	superBlock->iLock = false;
	writeCurrentBlock();
	blockReader->writeBlock(0, (char*)superBlock);
	delete superBlock;
	delete blockReader;
}
