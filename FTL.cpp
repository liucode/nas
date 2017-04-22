#include "FTL.h"
//FTL common

void FTL::printSTATE()
{
 printf("the number of moving:%d\nthe number of writing in log:%d\nthe number of cache hit:%d\nthe number of page writing:%d\nthe number of overwriting:%d\nthe number of translation block writing:%d\nthe number of find:%ld\nthe number of ex read:%d\nthe number of read bypass:%d\nthe number of page reading:%d\n",movenum,logwritenum,cachenum,pagewritenum,overwritenum,tblocknum,findnum,exreadnum,readbypass,pagereadnum);
}

char* FTL::readPBN(int pbn)
{
	pagereadnum++;
  if(DEBUG)
    printf("read pbn:%d\n",pbn);
  void *align_buf = NULL;
  if (posix_memalign(&align_buf,DATALEN,page_size) != 0) 
  {
        printf("memalign failed\n");
        assert(0);
  }
	long offset = (long)pbn*(long)page_size;
  pread(fp,align_buf,page_size/sizeof(char),offset);
  return (char*)align_buf;
}

int FTL::writePBN(int pbn,char *data)
{
  if(DEBUG)
    printf("write pbn:%d\n",pbn);
  pagewritenum++;
  long offset = (long)pbn*(long)page_size;
  if(pwrite(fp,data,page_size/sizeof(char),offset)!=4096)
  {
    printf("error writePBN\n");
    assert(0);
  }
  //free(data);
  //free(align_buf);
  return 1;
}



//PFTL 
int PFTL::writeFTL(int lbn,char *data)
{
	int pbn = findFreePBN();
	if(pbn == -1)
  {
    printf("error pbn used\n");
    gc();
    printSTATE();
    exit(0);
    return 0;
  }
  writePBN(pbn,data);
  if(DEBUG)
    printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  //gc 
  if(p_map[lbn]!=-1&&valid[p_map[lbn]]!=FREE)
  {
    if(DEBUG)
      printf("overwrite\n");
    overwritenum++;
    valid[p_map[lbn]] = INVALID;
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

//FINDFREE slow
int PFTL::findFreePBN()
{
	int i;
  for(i=rand()%page_num;i<2*page_num+pagefind;i++)
	{
	  findnum++;
		if(valid[i%page_num] == FREE)
		{
      pagefind = i%page_num;
			return i%page_num;
		}
	}
	return -1;
}

int PFTL::findPBN(int lbn)
{
	return p_map[lbn];
}
void PFTL::gc()
{
    int n=0;
    int sum=0;
    int i,j;
    int per_page = page_num/block_num;
    for(i=0;i<block_num;i++)
    {
      int num =0;
      for(j=0;j<per_page;j++)
      {
        if(valid[i*per_page+j]!=VALID)
          num++;
      }
      if(num>=0.5*per_page)
      {
          n++;
      }
      sum+=num;
    }
    printf("need gc %d %d %d\n",n,block_num,sum);
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
    }
    else
    {
      int pbn = findFreePBN();
      writePBN(pbn*per_page+o_lbn,data);//write in page
      p_valid[pbn*per_page+o_lbn] = LOG;
      if(DEBUG)
        printf("lbn-->pbn:%d-->%d\n",lbn,pbn*per_page+o_lbn); 
      b_map[b_lbn] = pbn;
      valid[pbn] = VALID;
 
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


      writeLOG(lbn,data);
      if(DEBUG)
        printf("write log：%d\n",lbn);
    }
  }
}
int BFTL::writeLOG(int lbn,char *data)
{
    logwritenum++;
    void *align_buf = NULL;
    if (posix_memalign(&align_buf,LBNLEN,LBNLEN)!= 0) 
    {
        printf("memalign failed\n");
        assert(0);
    }
    sprintf((char*)align_buf,"%0512d",lbn);//complement lbn
    /*if(write(logfp,align_buf,LBNLEN)==-1)
    {
        printf("error write log\n");
        assert(0);
    }
    */
    void *data_buf = NULL;
    if (posix_memalign(&data_buf,512,page_size)!= 0) 
    {
        printf("memalign failed\n");
        assert(0);
    }
    strcpy((char*)data_buf,data);
    if(write(logfp,data_buf,page_size)==-1)
    {
      printf("error write log data\n");
      assert(0);
    }
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
            exreadnum++;
            filedata = readPBN(oldblock*per_page+i);
            writePBN(newblock*per_page+i,filedata);
            p_valid[newblock*per_page+i] = VALID;
            p_valid[oldblock*per_page+i] = FREE;
          }
          else
          {
            writePBN(newblock*per_page+i,data);
            p_valid[newblock*per_page+i] = VALID;
            p_valid[oldblock*per_page+i] = FREE;
          }
        }
    }
    if(DEBUG)
      printf("move:%d-->%d\n",oldblock,newblock);
    valid[newblock] = VALID;
    valid[oldblock] = FREE;
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
	for(i=blockfind+1;i<block_num+blockfind;i++)
	{
		findnum++;
    if(valid[i%block_num] == FREE)
		{
      blockfind = i%block_num;
			return i%block_num;
		}
	}
  printf("no enough block\n");
	return -1;
}

int BFTL::findPBN(int lbn)
{
	return b_map[lbn];
}

//DFTL
void DFTL::liupread(int fp,char* data,int len,int offset)
{
    long newoffset = page_num;
    newoffset*=page_size;
    newoffset+=offset;
    pread(fp,data,len,newoffset);   
    return;
}
void DFTL::liupwrite(int fp,int data,int len,int offset)
{
    long newoffset = page_num;
    newoffset*=page_size;
    newoffset+=offset;
    void *align_buf = NULL;
    if (posix_memalign(&align_buf,LBNLEN,LBNLEN)!= 0) 
    {
        printf("memalign failed\n");
        assert(0);
    }
    sprintf((char*)align_buf,"%0512d",data);//complement lbn
    if(pwrite(fp,align_buf,LBNLEN,newoffset)==-1)
    {
      printf("error writePBN\n");
      assert(0);
    }
    return ;
}

int DFTL::writeFTL(int lbn,char *data)
{
  int pbn = findFreePagePBN();
  writePBN(pbn,data);
  if(DEBUG)
    printf("lbn-->pbn:%d-->%d\n",lbn,pbn);
  p_valid[pbn] = VALID;
  lnode findnode = HSLread(ht,cmt,lbn,ms);
  if(findnode != NULL)
  {
    p_valid[findnode->pbn] = INVALID;
    cmt->head->pbn = pbn;
    readflag[cmt->head->lbn] = FREE;
    if(DEBUG)
      printf("in cache %d\n",lbn);  
    cachenum++;
    return 1;
  }
  else
  {
    lnode m = HSLinsert(ht,cmt,lbn,pbn,ms);
    delNode(m);
  }
	return 1;
}

int DFTL::delNode(lnode m)
{  //if NULL,NO write 
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
      if(b_map[b_lbn] == -1)
      {
        int newpbn = findFreeBlockPBN();
        
        b_map[b_lbn] = newpbn;
        valid[newpbn] = VALID;
        int newpagepbn = m->pbn;

        tblocknum++;
        //liupwrite(tfp,newpagepbn,LBNLEN,per_page*newpbn*LBNLEN+o_lbn*LBNLEN);
        liupwrite(fp,newpagepbn,LBNLEN,per_page*newpbn*LBNLEN+o_lbn*LBNLEN);
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
          //liupwrite(tfp,newpagepbn,LBNLEN,per_page*bpbn*LBNLEN+o_lbn*LBNLEN);
          liupwrite(fp,newpagepbn,LBNLEN,per_page*bpbn*LBNLEN+o_lbn*LBNLEN);
          if(DEBUG)
            printf("add tblock: %d-->%d\n",per_page*bpbn+o_lbn,newpagepbn);
          tb_valid[bpbn*per_page+o_lbn] = VALID;
        }
        else
        {
          int newpbn = findFreeBlockPBN();
          void *data = NULL;
          posix_memalign(&data,LBNLEN,LBNLEN);
          int oldpbn = b_map[b_lbn];
          if(DEBUG)
          printf("move tblock:%d-->%d\n",oldpbn,newpbn);
          movenum++;
          for(int i=0;i<per_page;i++)
          {
              if(tb_valid[oldpbn*per_page+i] == VALID)
              {
                exreadnum++;
                char * data_t = (char *)data;
                liupread(fp,data_t,LBNLEN/sizeof(char),oldpbn*per_page*LBNLEN+i*LBNLEN);
                //liupwrite(tfp,atoi(data_t),LBNLEN/sizeof(char),newpbn*per_page*LBNLEN+i*LBNLEN);
                liupwrite(fp,atoi(data_t),LBNLEN/sizeof(char),newpbn*per_page*LBNLEN+i*LBNLEN);
                tb_valid[newpbn*per_page+i] = VALID;
                tb_valid[oldpbn*per_page+i] = FREE;
                tblocknum++;
              }//if
          }//for
          b_map[b_lbn] = newpbn;
          valid[oldpbn] = FREE;
          valid[newpbn] = VALID;
        }//else
      }//else
    }//else real
}

char* DFTL::readFTL(int lbn)
{ 
  int realpbn;
  if(DEBUG)
    printf("read lbn:%d\n",lbn);
  int o_lbn = lbn%per_page;
  int b_lbn = lbn/per_page;
  int pbn = findPBN(b_lbn);
  int tbpbn = pbn*per_page+o_lbn;
  void *tbdata = NULL;
  posix_memalign(&tbdata,LBNLEN,LBNLEN);
  char * tbdata_t = (char *)tbdata;
  int flag =1;
  lnode findnode = HSLread(ht,cmt,lbn,ms);
  if(findnode!=NULL)
  {
      realpbn = findnode->pbn;
      flag =2;
  }
  else
  {
   if(DEBUG)
      printf("read tbblock:%d\n",tbpbn);
    liupread(fp,tbdata_t,LBNLEN/sizeof(char),tbpbn*LBNLEN);
    realpbn = atoi(tbdata_t);
    readflag[lbn] = VALID;
    lnode m = HSLinsert(ht,cmt,lbn,realpbn,ms);
    if(readflag[m->lbn]==FREE)
      delNode(m);
    else
    {
      readbypass++;
      if(DEBUG)
        printf("read bypass:%d\n");
    }
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
  //add loc read
  if(flag==0)
  {
	  for(int i=1;i<6;i++)
    {
      liupread(fp,tbdata_t,LBNLEN/sizeof(char),(tbpbn+i)*LBNLEN);
      realpbn = atoi(tbdata_t);
      if(realpbn!=0&&p_valid[realpbn]!=FREE)
      {
        readflag[lbn+i] = VALID;
        lnode m = HSLinsert(ht,cmt,lbn,realpbn,ms);
        if(readflag[m->lbn]==FREE)
          delNode(m);
      }
    }
  }
  return data;
}

int DFTL::findFreePagePBN()
{
	int i;
	for(i=rand()%page_num;i<2*page_num+pagefind;i++)
	{
    findnum++;
		if(p_valid[i%page_num] == FREE)
		{
      pagefind = i%page_num;
			return i%page_num;
		}
	}
	return -1;
}

int DFTL::findFreeBlockPBN()
{
  int i;
	for(i=blockfind+1;i<block_num+blockfind;i++)
	{
		findnum++;
    if(valid[i%block_num] == FREE)
		{
      blockfind = i%block_num;
      return i%block_num;
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
  if(p_map[lbn] != -1)//overwrite
  {
      overwritenum++;
      if(DEBUG)
        printf("overwrite lbn:%d\n",lbn);
      int oldpbn = findTruePBN(lbn,p_map[lbn]);
      //lvalid[oldpbn/per_page]=lvalid[oldpbn/per_page]&(~(1<<oldpbn%per_page));
      p_valid[oldpbn] = INVALID;
  }
  int truepbn;
  int shortpbn = findFreePBN(lbn,truepbn);
  if(shortpbn == -1)//not found
  {
     tblocknum++; 
     if(DEBUG)
        printf("not found free page\n");
      for(int i=rand()%hlen;i<2*hlen+pagefind;i++)
      {
        i = i%hlen;
        findnum++;
        //if(i%per_page==0&&p_valid[i] == FREE||p_valid[i] == FREE&&p_valid[i-1] != FREE)
        //if(i%per_page==0&&(lvalid[i/per_page]&(1<<(i%per_page)))==0||((lvalid[i/per_page]&(1<<(i%per_page)))==0)&&((lvalid[i/per_page]&(1<<(i%per_page-1)))!=0))
        if(e_valid[i]==FREE)
        {
          truepbn = i;
          pagefind = i;
          break;
        }
      }
      HSinsert(cmt,lbn,truepbn,HASHLEN);
      writePBN(truepbn+page_num,data);
      if(DEBUG)
        printf("write log lbn-->pbn:%d-->%d\n",lbn,truepbn);
      p_map[lbn] = (pow(2,khn)-1)*pow(2,mon);
      e_valid[truepbn] = VALID;
      //lvalid[truepbn/per_page]+=(1<<(truepbn%per_page));
  }
  else
  {
    if(truepbn!=-1)
    {
      writePBN(truepbn,data);
    }
    else
    {
      assert(0);
    }
    if(DEBUG)
      printf("lbn-->pbn:%d-->%d\n",lbn,truepbn);
    p_map[lbn] = shortpbn;
    p_valid[truepbn] = VALID;
    //lvalid[truepbn/per_page]+=(1<<(truepbn%per_page));
  }
	return 1;
}

char* HFTL::readFTL(int lbn)
{
	int truepbn;
  int pbn = findPBN(lbn);
  if(DEBUG)
    printf("read LBN:%d\n",lbn);
  if(pbn/pow(2,mon)==pow(2,khn)-1)
  {
    hnode p = HSfind(cmt,lbn,HASHLEN);
    truepbn = p->pbn+page_num;
  }
  else
  {
    truepbn = findTruePBN(lbn,pbn);
  }
	char *data;
  if(p_valid[truepbn] == VALID)
  //if(lvalid[truepbn/per_page]&(1<<(truepbn%per_page))!=0)
  {
    data = readPBN(truepbn);
  }
  else
  {
    data = new char[10];
    strcpy(data,"not found\n");
  }
	return data;
}
int HFTL::findFreePBN(int lbn,int &tpbn)
{
	int i,j;
  int templbn = lbn;
  for(i=0;i<pow(2,khn)-1;i++)
	{
		  int k = templbn>>i;
      k = k%block_num;
      for(j=0;j<pow(2,mon);j++)
      {
        findnum++;
        int m = per_page/pow(2,mon);//aslo need m 
        int offset = j*m+(lbn%m);
        if(p_valid[k*per_page+offset-1] == FREE&&offset!=0||p_valid[(k+1)*per_page-1]!=FREE)
        //if((lvalid[k]&(1<<(offset-1)))==0&&offset!=0)
        {
           break;
        } 
        if(offset == 0&&p_valid[k*per_page+offset] == FREE||p_valid[k*per_page+offset] == FREE&&p_valid[k*per_page+offset-1] != FREE)
        //if(offset ==0&&(lvalid[k]&(1<<offset))==0||(lvalid[k]&(1<<(offset-1)))!=0&&(lvalid[k]&(1<<offset))==0)
        //if(true)
        {
          tpbn = k*per_page+offset;
          return i*pow(2,mon)+j;
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
   int templbn = lbn;
   int k = pbn/pow(2,mon);
   k  = templbn>>k;
   k = k%block_num;
   int m = per_page/pow(2,mon);//aslo need m 
   int tpmon = pow(2,mon);
   int offset = pbn%tpmon*m+lbn%m;
   return k*per_page+offset;
}

void HFTL::gc()
{
    int i;
    int pbn;
    hnode prep;
    int num=0;
    int nonum =0;
    for(i=0;i<HASHLEN;i++)
    {
         hnode p = cmt[i];
         prep = p;
         int truepbn;
         while(p)
         {
            pbn  = findFreePBN(p->lbn,truepbn);
            if(pbn == -1)// don't need handle
            {
                nonum++;
            }
            else//new pbn
            {
              num++;  
              p_map[p->lbn] = truepbn;
              p_valid[p->pbn] = INVALID;
              p_valid[truepbn] = VALID;
              char *data;
              data = readPBN(p->pbn);
              writePBN(truepbn,data);
              if(p->lbn == cmt[i]->lbn)
              {
                cmt[i] = p->next;
              }
              else
              {
                prep->next = p->next;
              }
            }
            prep = p;
            p = p->next;
         }
    }
    printf("gc no num and num:%d %d\n",nonum,num);
}
