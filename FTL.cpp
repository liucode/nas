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
      OOB[pbn*per_page] = lbn;
    }
    else
    {
      int pbn = findFreePBN();
      writePBN(pbn*per_page,data);//write in page
      p_valid[pbn*per_page] = VALID;
      printf("lbn-->pbn:%d-->%d\n",lbn,pbn*per_page); 
      b_map[b_lbn] = pbn;
      valid[pbn] = VALID;
      OOB[pbn*per_page] = lbn;
 
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
      OOB[realpbn+o_lbn] = lbn;

    }
    //same overwrite?
    else if(p_valid[realpbn+o_lbn-1]!=FREE&&p_valid[realpbn+o_lbn] != FREE)
    {
      printf("lbn:%d overwrite\n",lbn);
      movePBN(lbn,data); 
    }
    else
    {
      writePBN(realpbn+o_lbn,data);
      printf("lbn-->pbn:%d-->%d\n",lbn,realpbn+o_lbn);
      p_valid[realpbn+o_lbn] = VALID;
      OOB[realpbn+o_lbn] = lbn;


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

int BFTL::movePBN(int lbn,char *data)
{ 
    int o_lbn = lbn%64;
    int b_lbn = lbn/64;
  
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
            OOB[newblock*per_page+i] = OOB[oldblock*per_page+i];
          }
          else
          {
            writePBN(newblock*per_page+i,data);
            OOB[newblock*per_page+i] = lbn;
          }
        }
    }
    printf("move:%d-->%d\n",oldblock,newblock);
    return 1;
}

//no complete
char* BFTL::readFTL(int lbn)
{
  int o_lbn = lbn%64;
  int b_lbn = lbn/64;
  
	int pbn = findPBN(b_lbn);
	char *data;
  if(valid[pbn] == VALID)
  {
    data = readPBN(pbn*per_page+o_lbn);
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

//DFTL
int DFTL::writeFTL(int lbn,char *data)
{
  int pbn = findFreePagePBN();
	lnode findnode = LRUread(list,lbn);
  if(findnode != NULL)
  {
    findnode->pbn = pbn;
    p_valid[findnode->pbn] = Free;
  }
  lnode m = LRUinsert(cmt,lbn,pbn);
  writePBN(pbn,data);
  printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  valid[pbn] = VALID;
  //if NULL,NO write 
  if(m == NULL)
  {
      return 1;
  }
  else//really write 
  {
      int o_lbn = lbn%64;
      int b_lbn = lbn/64;
      //first
	    char pbnstr[LBNLEN];
      if(b_map[b_lbn] == -1)
      {
        int newpbn = findFreeBlockPBN();
        b_map[b_lbn] = newpbn;
        sprintf(pbnstr,"%032",newpbn);//complement lbn
        pwrite(fp,pbnstr,LBNLEN,per_page*newpbn*LBNLEN+o_lbn*LBNLEN);
      }
      else
      {
        int bpbn = b_map[b_lbn];
        if(p_valid[bpbn*per_page] == FREE)
        {
          sprintf(pbnstr,"%032",pbn);//complement lbn
          pwrite(fp,pbnstr,LBNLEN,per_page*bpbn*LBNLEN+o_lbn*LBNLEN);
        }
        else
        {
          int newpbn = findFreeBlockPBN();
          char *data;
          data = new char[LBNLEN/sizeof(char)];
          int oldpbn = b_map[b_lbn];
          printf("move tblock:%d-->%d\n",oldpbn,newpbn);
          for(int i=0;i<per_page;i++)
          {
            if(i!=o_lbn)
            {
              pread(tfp,data,LBNLEN/sizeof(char),oldpbn*per_page*LBNLEN+i);
              pwrite(tfp,data,LBNLEN/sizeof(char),newpbn*per_page*LBNLEN+i);
            }
            else
            {
               pread(tfp,data,LBNLEN/sizeof(char),oldpbn*per_page*LBNLEN+i);
               p_valid[atoi(data)] = FREE;

               sprintf(pbnstr,"%032",pbn);//complement lbn
               pwrite(tfp,,LBNLEN/sizeof(char),newpbn*per_page*LBNLEN+i);
            }
          }
        }
      }
  }
	return 1;
}

char* DFTL::readFTL(int lbn)
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

int DFTL::findFreePagePBN()
{
	int i;
	for(i=0;i<page_num;i++)
	{
		if(p_valid[i] == FREE)
		{
			return i;
		}
	}
	return -1;
}

int DFTL::findFreeBlockPBN()
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

int DFTL::findPBN(int lbn)
{
	return p_map[lbn];
}


