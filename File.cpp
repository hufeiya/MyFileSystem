#include "File.h"



File::File()
{
	fcb = nullptr;
}

File::File(string path)
{
	fcb = nullptr;
	this->path = path;
}

string File::read(short start, short end)
{
	return string();
}

void File::write(short start, string content)
{
}


File::~File()
{
}
