#pragma once
#include <iostream>
#include <iomanip>
#include "CatalogItem.h"
using namespace std;
class Catalog
{
public:
	CatalogItem items[32];
	Catalog();
	void print();
	~Catalog();
};

