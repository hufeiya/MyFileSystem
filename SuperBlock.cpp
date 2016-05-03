#include "SuperBlock.h"



SuperBlock::SuperBlock()
{
}

/**
*	��ʼ��������,���ڴ���ʹ�ó��ε���
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
*	��ӡ��������Ϣ
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

/**	�����̿��	*/
short SuperBlock::allocBlock()
{
	if (modifySignal)
	{
		cerr << "����������ʹ����,���Ժ�����" << endl;
		return NONE;
	}
	else if (freeBlockNum <= 0)
	{
		cerr << "���޿����̿�,���Ժ�����" << endl;
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

/**	�����̿�	*/
void SuperBlock::retrieveBlock(short blockNum)
{
	if (modifySignal)
	{
		cerr << "����������ʹ����,���Ժ�����" << endl;
	}
	else if (freeBlockNum >= BLOCK_NUM)
	{
		cerr << "�̿���ȫ������,�����ٻ���" << endl;
	}
	else
	{
		modifySignal = true;
		blockStack[freeBlockNum++] = blockNum;
		modifySignal = false;
	}
}

/**	����FCB	*/
short SuperBlock::allocFCB()
{
	if (modifySignal)
	{
		cerr << "����������ʹ����,���Ժ�����" << endl;
		return NONE;
	}
	else if (inodeNum <= 0)
	{
		cerr << "���޿���������,���Ժ�����" << endl;
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

/**	����FCB	*/
void SuperBlock::retrieveFCB(short fcbNum)
{
	if (modifySignal)
	{
		cerr << "����������ʹ����,���Ժ�����" << endl;
	}
	else if (inodeNum >= 40)
	{
		cerr << "��������ȫ������,�����ٻ���" << endl;
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
