#pragma once
class UserItem
{
public:
	short id;
	short groupNum;
	char name[14];
	char home[14];
	char password[32];

	UserItem();
	~UserItem();
};

