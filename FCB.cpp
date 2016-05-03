#include "FCB.h"



FCB::FCB()
{
}

/**¸³Ä¬ÈÏÖµ*/
void FCB::mask()
{
	authority[0] = 7;
	authority[1] = 4;
	authority[2] = 4;
	length = 512;
	accessCount = 0;
	time_t newTime;
	time(&newTime);
	modifyTime = (int)newTime;
	owner = 0;
	group = 0;
	linkNum = 0;
}

void FCB::print()
{
	cout << "num " << num << endl;
	cout<<"type "<<int(type)<<endl;
	cout << "authority[0] " << int(authority[0]) << endl;
	cout << "authority[1] " << int(authority[1]) << endl;
	cout << "authority[2] " << int(authority[2]) << endl;
	cout << "length " << length << endl;
	cout << "accessCount " << accessCount << endl;
	cout << "modifyTime " << modifyTime << endl;
	cout << "owner " << int(owner) << endl;
	cout << "group " << group << endl;
	cout << "linkNum " << linkNum << endl;
	cout << " address_0 " << address[0] << endl;
}


FCB::~FCB()
{
}
