#include "FTL.h"
//FTL common
void FTL::printSTATE()
{
  printf("the number of moving:%d\nthe number of writing in log:%d\nthe number of cache hit:%d\nthe number of page writing:%d\nthe number of overwriting:%d\nthe number of translation block writing:%d\n",movenum,logwritenum,cachenum,pagewritenum,overwritenum,tblocknum);
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
  pagewritenum++;
  pwrite(fp,data,page_size/sizeof(char),pbn*page_size);
	return 1;
}



//PFTL 
int PFTL::writeFTL(int lbn,char *data)
{
	int pbn = findFreePBN();
	writePBN(pbn,data);
  if(DEBUG)
  printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  //gc 
  if(p_map[lbn]!=-1&&valid[p_map[lbn]]!=FREE)
  {
    if(DEBUG)
    printf("overwrite\n");
    overwritenum++;
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
  int o_lbn = lbn%per_page;
  int b_lbn = lbn/per_page;
  if(b_map[b_lbn]==-1)//no block mapping
  {
    if(o_lbn == 0)//first
    {
      int pbn = findFreePBN();
      writePBN(pbn*per_page,data);//write in page
      p_valid[pbn*per_page] = VALID;
      if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,pbn*per_page); 
      b_map[b_lbn] = pbn;
      valid[pbn] = VALID;
      OOB[pbn*per_page] = lbn;
    }
    else
    {
      int pbn = findFreePBN();
      writePBN(pbn*per_page,data);//write in page
      p_valid[pbn*per_page+o_lbn] = LOG;
      if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,pbn*per_page+o_lbn); 
      b_map[b_lbn] = pbn;
      valid[pbn] = VALID;
      OOB[pbn*per_page] = lbn;
 
      writeLOG(lbn,data);
      if(DEBUG)
      printf("write log：%d\n",lbn);
    }
  }
  else
  {
    int realpbn = b_map[b_lbn]*per_page;
    if(p_valid[realpbn+o_lbn-1]==VALID&&p_valid[realpbn+o_lbn] == FREE)
    {
      writePBN(realpbn+o_lbn,data);
      if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,realpbn+o_lbn);
      p_valid[realpbn+o_lbn] = VALID;
      OOB[realpbn+o_lbn] = lbn;
    }
    //same overwrite?
    else if(p_valid[realpbn+o_lbn] != FREE)
    {
      if(DEBUG)
      printf("lbn:%d overwrite\n",lbn);
      movenum++;
      movePBN(lbn,data); 
    }
    else
    {
      writePBN(realpbn+o_lbn,data);
      if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,realpbn+o_lbn);
      p_valid[realpbn+o_lbn] = LOG;
      OOB[realpbn+o_lbn] = lbn;


      writeLOG(lbn,data);
      if(DEBUG)
      printf("write log：%d\n",lbn);
    }
  }
}
int BFTL::writeLOG(int lbn,char *data)
{
    logwritenum++;
    char lbnstr[LBNLEN];
    sprintf(lbnstr,"%032",lbn);//complement lbn
    write(logfp,lbnstr,LBNLEN);
    write(logfp,data,page_size);
    return 1;
}

int BFTL::movePBN(int lbn,char *data)
{ 
    int o_lbn = lbn%per_page;
    int b_lbn = lbn/per_page;
  
    valid[b_map[b_lbn]] = INVALID;
    int oldblock = b_map[b_lbn];
    int newblock = findFreePBN();
    for(int i=0;i<per_page;i++)
    {
        char *filedata;
        if(p_valid[oldblock*per_page+i]==VALID||p_valid[oldblock*per_page+i]==LOG)
        {
          if(i!=o_lbn)
          {
            filedata = readPBN(oldblock*per_page+i);
            writePBN(newblock*per_page+i,filedata);
            p_valid[newblock*per_page+i] = VALID;
            p_valid[oldblock*per_page+i] = FREE;
            OOB[newblock*per_page+i] = OOB[oldblock*per_page+i];
          }
          else
          {
            writePBN(newblock*per_page+i,data);
            OOB[newblock*per_page+i] = lbn;
            p_valid[newblock*per_page+i] = VALID;
            p_valid[oldblock*per_page+i] = FREE;
          }
        }
    }
    if(DEBUG)
    printf("move:%d-->%d\n",oldblock,newblock);
    valid[newblock] = VALID;
    b_map[b_lbn] = newblock;
    return 1;
}

//no complete
char* BFTL::readFTL(int lbn)
{
  int o_lbn = lbn%per_page;
  int b_lbn = lbn/per_page;
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
  writePBN(pbn,data);
  if(lbn==917&&pbn==213)
  {
    int k =1;
  }
  if(DEBUG)
  printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  p_valid[pbn] = VALID;
  lnode findnode = LRUread(cmt,lbn,ms);
  if(findnode != NULL)
  {
    p_valid[findnode->pbn] = FREE;
    cmt->head->pbn = pbn;
    if(DEBUG)
    printf("in cache %d\n",lbn);  
    cachenum++;
    return 1;
  }
  else
  {
    lnode m = LRUinsert(cmt,lbn,pbn,ms);
  //if NULL,NO write 
    if(m == NULL)
    {
      return 1;
    }
    else//really write 
    {
      int o_lbn = m->lbn%per_page;
      int b_lbn = m->lbn/per_page;
      if(DEBUG)
      printf("delete CMT lbn-->pbn:%d-->%d\n",m->lbn,m->pbn);
      //first
	    char pbnstr[LBNLEN];
      if(b_map[b_lbn] == -1)
      {
        int newpbn = findFreeBlockPBN();
        b_map[b_lbn] = newpbn;
        valid[newpbn] = VALID;
        int newpagepbn = m->pbn;

        tblocknum++;
        sprintf(pbnstr,"%032",newpagepbn);//complement lbn
        pwrite(fp,pbnstr,LBNLEN,per_page*newpbn*LBNLEN+o_lbn*LBNLEN);
        
        if(DEBUG)
        printf("new %d block\n",newpbn); 
        if(DEBUG)
        printf("tblock: %d-->%d\n",per_page*newpbn+o_lbn,newpagepbn);
        tb_valid[per_page*newpbn+o_lbn] = VALID;
      }
      else
      {
        int bpbn = b_map[b_lbn];
        if(tb_valid[bpbn*per_page+o_lbn] == FREE)
        {
          int newpagepbn = m->pbn;

          tblocknum++;
          sprintf(pbnstr,"%032",newpagepbn);//complement lbn
          pwrite(fp,pbnstr,LBNLEN,per_page*bpbn*LBNLEN+o_lbn*LBNLEN);
          if(DEBUG)
          printf("tblock: %d-->%d\n",per_page*bpbn+o_lbn,newpagepbn);
          tb_valid[bpbn*per_page+o_lbn] = VALID;
        }
        else
        {
          int newpbn = findFreeBlockPBN();
          char *data;
          data = new char[LBNLEN/sizeof(char)];
          int oldpbn = b_map[b_lbn];
          if(DEBUG)
          printf("move tblock:%d-->%d\n",oldpbn,newpbn);
          movenum++;
          for(int i=0;i<per_page;i++)
          {
              if(tb_valid[oldpbn*per_page+i] == VALID)
              {
                pread(tfp,data,LBNLEN/sizeof(char),oldpbn*per_page*LBNLEN+i*LBNLEN);
                pwrite(tfp,data,LBNLEN/sizeof(char),newpbn*per_page*LBNLEN+i*LBNLEN);
                tb_valid[newpbn*per_page+i] = VALID;
                tb_valid[oldpbn*per_page+i] = FREE;
                tblocknum++;
              }
          }//for
          b_map[b_lbn] = newpbn;
          valid[oldpbn] = FREE;
          valid[newpbn] = VALID;
        }//else
      }
    }
        
  }
	return 1;
}

char* DFTL::readFTL(int lbn)
{ 
  int realpbn;
  lnode findnode = LRUread(cmt,lbn,ms);
  if(findnode!=NULL)
  {
      realpbn = findnode->pbn;
  }
  else
  {
    int o_lbn = lbn%per_page;
    int b_lbn = lbn/per_page;
    int pbn = findPBN(b_lbn);
	  char *data;
    realpbn = pbn*per_page+o_lbn;
  }
  
  char *data = new char[10];
  if(p_valid[realpbn] !=FREE)
  {  
    data = readPBN(realpbn);
  }
  else
  {
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
	return b_map[lbn];
}


//HFTL
int HFTL::writeFTL(int lbn,char *data)
{
	int shortpbn = findFreePBN();
  if(shortpbn == -1)//not found
  {
      if(DEBUG)
        printf("not found free page\n");
      for(int i=0;i<page_num;i++)
      {
        if(p_valid[i] == FREE)
        {
          truepbn = i;
          break;
        }
      }
      NMinsert(cmt,lbn,pbn);
  }
  else
  {
    if(truepbn!=0)
    {
      writePBN(truepbn,data);
      OOB[truepbn] = lbn;
    }
    if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,truepbn);
    p_map[lbn] = shortpbn;
    p_valid[pbn] = VALID;
  }
	return 1;
}

char* HFTL::readFTL(int lbn)
{
	int pbn = findPBN(lbn);
  truepbn = findTruePBN(pbn);
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
int HFTL::findFreePBN(int lbn)
{
	int i,j;
	for(i=0;i<power(2,khn);i++)
	{
		  int k = lbn>>i;
      k = k%block_num;
      for(j=0;j<pow(2,mon);j++)
      {
        int m = per_page/power(2,mon);//aslo need m 
        int offset = j*m+(lbn%m);
        if(OOB[k*per_page+offset] == lbn)//overwrite
        {
            overwritenum++;
            if(DEBUG)
              printf("overwrite lbn-->pbn\n",truepbn,lbn);
            p_map[k*per_page+offset] = FREE;
            truepbn = k*per_page+offset;
            return i*power(2,mon)+j;
        }

        if(p_map[k*per_page+offset] == FREE)
        {
          truepbn = k*per_page+offset;
          return i*power(2,mon)+j;
        }
      }
     }
	return -1;
}

int HFTL::findPBN(int lbn)
{
	return p_map[lbn];
}

int HFTL::findTruePBN(int lbn,int pbn)
{
   int k = pbn/power(2,mon);
   k  = lbn>>k;
   k = k%block_num;
   int m = per_page/power(2,mon);//aslo need m 
   int offset = pbn%power(2,mon)*m+lbn%m;
   return k*per_page+offset;
}

