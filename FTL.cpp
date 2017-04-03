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
//lbn limited(the same overwrite)
int BFTL::writeFTL(int lbn,char *data)
{
  int o_lbn = lbn%64;
  int b_lbn = lbn/64;
  if(b_map[b_lbn]==-1)//no block mapping
  {
    if(o_lbn == 0)//first
    {
      int pbn = findFreePBN();
      writePBN(pbn*per_page,data);//write in page
      p_valid[pbn*per_page] = VALID;
      printf("lbn-->pbn:%d-->%d\n",lbn,pbn*per_page); 
      b_map[b_lbn] = pbn;
      valid[pbn] = VALID;
    }
    else
    {
      writeLOG(lbn,data);
      printf("write log：%d\n",lbn);
    }
  }
  else
  {
    int realpbn = b_map[b_lbn]*per_page;
    if(p_valid[realpbn+o_lbn-1]!=FREE&&p_valid[realpbn+o_lbn] == FREE)
    {
      writePBN(realpbn+o_lbn,data);
      printf("lbn-->pbn:%d-->%d\n",lbn,realpbn+o_lbn);
      p_valid[realpbn+o_lbn] = VALID;

    }
    else if(p_valid[realpbn+o_lbn-1]!=FREE&&p_valid[realpbn+o_lbn] != FREE)
    {
      printf("lbn:%d overwrite\n",lbn);
      movePBN(b_lbn,o_lbn,data); 
    }
    else
    {
      writeLOG(lbn,data);
      printf("write log：%d\n",lbn);
    }
  }
}
int BFTL::writeLOG(int lbn,char *data)
{
    char lbnstr[LBNLEN];
    sprintf(lbnstr,"%032",lbn);//complement lbn
    write(logfp,lbnstr,LBNLEN);
    write(logfp,data,page_size);
    return 1;
}

int BFTL::movePBN(int b_lbn,int o_lbn,char *data)
{
    valid[b_map[b_lbn]] = INVALID;
    int oldblock = b_map[b_lbn];
    int newblock = findFreePBN();
    for(int i=0;i<per_page;i++)
    {
        char *filedata;
        if(p_valid[oldblock*perpage+i]==VAILD)
        {
          if(i!=o_lbn)
          {
            filedata = readPBN(oldblock*per_page+i);
            writePBN(newblock*per_page+i,filedata);
          }
          else
          {
            writePBN(newblock*per_page+i,data);
          }
        }
    }
    printf("move:%d-->%d\n",oldblock,newblock);
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
	for(i=0;i<block_num;i++)
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
	return b_map[lbn];
}




