#include "Catalog.h"



Catalog::Catalog()
{
}

void Catalog::print()
{
	for (int i = 0; i < 32; i++)
	{
		if (items[i].inode == 0)
		{
			break;
		}
		cout << left << setw(14) << items[i].fileName;
	}
}


Catalog::~Catalog()
{
}
