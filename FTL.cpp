#include "FTL.h"

//FTL common
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



//PFTL 
int PFTL::writeFTL(int lbn,char *data)
{
	int pbn = findFreePBN();
	writePBN(pbn,data);
  printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  //gc 
  if(p_map[lbn]!=-1&&valid[p_map[lbn]]!=FREE)
  {
    printf("overwrite\n");
    valid[p_map[lbn]] = FREE;
  }
  p_map[lbn] = pbn;
	valid[pbn] = VALID;
	return 1;
}

char* PFTL::readFTL(int lbn)
{
	int pbn = findPBN(lbn);
	char *data;
  if(valid[pbn] == VALID)
  {
    data = readPBN(pbn);
  }
  else
  {
    data = new char[10];
    strcpy(data,"not found\n");
  }
	return data;
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

//BFTL
int BFTL::writeFTL(int lbn,char *data)
{
	int pbn = findFreePBN();
	writePBN(pbn,data);
  printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  //gc 
  if(p_map[lbn]!=-1&&valid[p_map[lbn]]!=FREE)
  {
    printf("overwrite\n");
    valid[p_map[lbn]] = FREE;
  }
  p_map[lbn] = pbn;
	valid[pbn] = VALID;
	return 1;
}

char* BFTL::readFTL(int lbn)
{
	int pbn = findPBN(lbn);
	char *data;
  if(valid[pbn] == VALID)
  {
    data = readPBN(pbn);
  }
  else
  {
    data = new char[10];
    strcpy(data,"not found\n");
  }
	return data;
}
int BFTL::findFreePBN()
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

int BFTL::findPBN(int lbn)
{
	return p_map[lbn];
}




