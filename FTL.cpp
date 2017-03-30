#include "FTL.h"

int FTL::writeFTL(int lbn,char *data)
{
	int pbn = findFreePBN();
	writePBN(pbn,data);
	valid[pbn] = VALID;
	return 1;
}

char* FTL::readFTL(int lbn)
{
	int pbn = findPBN(lbn);
	char* data = readPBN(pbn);
	return data;
}

char* FTL::readPBN(int pbn)
{
	char *data;
	data = new char[page_size/sizeof(char)];
	pread(fp,data,page_size/sizeof(char),pbn*page_size);
  return data;
}

int FTL::writePBN(int pbn,char *data)
{
  pwrite(fp,data,page_size/sizeof(char),pbn*page_size);
	return 1;
}

int FTL::findFreePBN()
{
	return 1;
}
int FTL::findPBN(int pbn)
{
	return 1;
}


int PFTL::findFreePBN()
{
	int i;
	for(i=0;i<page_num;i++)
	{
		if(valid[i] == FREE)
		{
			return i;
		}
	}
	return -1;
}

int PFTL::findPBN(int lbn)
{
	return p_map[lbn];
}
