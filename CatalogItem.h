#pragma once
class CatalogItem
{
public:
	char fileName[14];
	short inode;
	CatalogItem();
	~CatalogItem();
};

