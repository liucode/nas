#include "SSDsim.h"
//init
SSD::SSD(int ds,int bs, int ps, int ms,int khn,int mon,int p):
disk_size(ds<<GB2KB),block_size(bs),page_size(ps),mem_size(ms),k_hash_num(khn),m_offset_num(mon),policy(p)
{

	 int block_num = disk_size/block_size;
	 int page_num = disk_size/page_size;
   
   //mem state
   int pftl_level = page_num*4/1024;
   int bftl_level = block_num*4/1024;
   
   int dftl_level = bftl_level;//second size
   int dnum  = (mem_size - dftl_level)/4*1024;//first num;

   int hftl_level = (k_hash_num+m_offset_num)*page_num/8/1024;//first
   int hftl_len = (mem_size-hftl_level)/4*1024;

   printf("page:%d\nblock:%d\nDFTL cache num:%d\nHFTL table num:%d\n",pftl_level,bftl_level,dnum,hftl_len);
   
   page_size = page_size<<KB2B;
   block_size = block_size<<KB2B;
   
   switch(policy)
	 {			
		  case PFTLNUM:
        {
          PFTL *pftl = new PFTL(page_num,page_size,block_num);
	        myftl = pftl;
        }
        break;
      case BFTLNUM:
        {
          BFTL *bftl = new BFTL(block_num,block_size,page_num,page_size);
          myftl = bftl;
        }
        break;
      case DFTLNUM:
        {
          DFTL *dftl = new DFTL(block_num,block_size,page_num,page_size,dnum);
          myftl = dftl;
        }
        break;
       case HFTLNUM:
       {
          HFTL *hftl = new HFTL(block_num,block_size,page_num,page_size,mem_size,khn,mon,hftl_len);
          myftl = hftl;
        }
        break;
        
	 }
}
uint64_t
debug_time_usec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000lu + tv.tv_usec;
}

double
debug_time_sec(void)
{
    const uint64_t usec = debug_time_usec();
    return ((double)usec);
}
void SSD::writeSSD(int lbn,char* data)//data struct
{
 	myftl->writeFTL(lbn,data);
}
char* SSD::readSSD(int lbn)
{
  char *data;
  data = myftl->readFTL(lbn);
  return data;
}
void SSD::zfTest(int n)
{
    int i;
    if(n==0)
      n = disk_size/page_size*1024/2;
    int *lbns = new int[n];
    char *data = new char[page_size/sizeof(char)];
		memset(data,'5',page_size/sizeof(char));
		void *align_buf = NULL;
  	if (posix_memalign(&align_buf,DATALEN,page_size) != 0)  
  	{
        printf("memalign failed\n");
        assert(0);
  	}
		strcpy((char *)align_buf,data);
    double starts,ends;
    double s,e;
    struct GenInfo * gis;
    gis = generator_new_zipfian(0, disk_size/page_size*1024);
    int lbn;
    for(i=0;i<n;i++)
    {
      while(true)
      {
        lbn = gis->next(gis);
        if(lbn>10)
            break;
      }
      lbns[i] = lbn;
    }
    s = debug_time_sec();
    for(i=0;i<n;i++)
    {
      //srand(i);
      //int lbn = rand()%(disk_size/page_size*1024);
      //lbn = mymd5(lbn)%(disk_size/page_size*1024);
      //int lbn = rand()%n;
      lbn = lbns[i];
      if(i%100==0)
        starts = debug_time_sec();
      writeSSD(lbn,(char *)align_buf);
      if((i+1)%100==0)
      {
        ends = debug_time_sec();
        printf("%d %f %d\n",i,ends-starts,myftl->findnum);
      }
    }
    for(i=0;i<n;i++)
    {
      //readSSD(lbns[i]);
      //printf("%d:%s\n",lbns[i],readSSD(lbns[i]));
    }
    fsync(myftl->fp);
    e = debug_time_sec();
    printf("time:%f\n",e-s);
    printf("random test ok\n");
}

void SSD::fileTest(int n,char* name)
{
    int i;
    if(n==0)
      n = disk_size/page_size*1024;
    int *lbns = new int[n];
    int *rws = new int[n];
    char *data = new char[page_size/sizeof(char)];
		memset(data,'5',page_size/sizeof(char));
		void *align_buf = NULL;
  	if (posix_memalign(&align_buf,DATALEN,page_size) != 0)  
  	{
        printf("memalign failed\n");
        assert(0);
  	}
		strcpy((char *)align_buf,data);
    double starts,ends;
    double s,e;
    int lbn;
    char rw[2];
    char rlbn[20];
    printf("name :%s\n",name);
    FILE *readfp = fopen(name,"r");
    if(readfp==NULL)
    {
      printf("open error\n");
      assert(0);
    }
    for(i=0;i<n;i++)
    {
      if(feof(readfp))
        n = i;
      fscanf(readfp,"%s",rw);
      fscanf(readfp,"%s",rlbn);
      rws[i] = atoi(rw);
      lbns[i] = atoi(rlbn);
    }
    s = debug_time_sec();
    for(i=0;i<n;i++)
    {
      //srand(i);
      //lbn = mymd5(lbn)%(disk_size/page_size*1024-1);
      //int lbn = rand()%n;
      lbn = lbns[i];
      if(i%100==0)
        starts = debug_time_sec();
      if(rws[i]==1)
        writeSSD(lbn,(char *)align_buf);
      else
        readSSD(lbn);
      if((i+1)%100==0)
      {
        ends = debug_time_sec();
        printf("%d %lf %d\n",i,ends-starts,myftl->findnum);
      }
    }
    //myftl->gc();
    fsync(myftl->fp);
    e = debug_time_sec();
    printf("time:%f\n",e-s);
    for(i=0;i<n;i++)
    {
      //readSSD(lbns[i]);
      //printf("%d:%s\n",lbns[i],readSSD(lbns[i]));
    }
    printf("random test ok\n");
}

void SSD::randomTest(int n)
{
    int i;
    if(n==0)
      n = disk_size/page_size*1024;
    int *lbns = new int[n];
    char *data = new char[page_size/sizeof(char)];
		memset(data,'5',page_size/sizeof(char));
		void *align_buf = NULL;
  	if (posix_memalign(&align_buf,DATALEN,page_size) != 0)  
  	{
        printf("memalign failed\n");
        assert(0);
  	}
		strcpy((char *)align_buf,data);
    double starts,ends;
    double s,e;
    int lbn;
    for(i=0;i<n;i++)
    {
      lbns[i] = rand()%(disk_size/page_size*1024-1);
    }
    s = debug_time_sec();
    for(i=0;i<n;i++)
    {
      //srand(i);
      //lbn = mymd5(lbn)%(disk_size/page_size*1024-1);
      //int lbn = rand()%n;
      lbn = lbns[i];
      if(i%100==0)
        starts = debug_time_sec();
      writeSSD(lbn,(char *)align_buf);
      if((i+1)%100==0)
      {
        ends = debug_time_sec();
        printf("%d %lf %d\n",i,ends-starts,myftl->findnum);
      }
    }
    //myftl->gc();
    fsync(myftl->fp);
    e = debug_time_sec();
    printf("time:%f\n",e-s);
    for(i=0;i<n;i++)
    {
      //readSSD(lbns[i]);
      //printf("%d:%s\n",lbns[i],readSSD(lbns[i]));
    }
    printf("random test ok\n");
}
void SSD::srTest(int r,int n)
{
  int i;
  time_t starts,ends;
  if(n==0)
    n = disk_size/page_size*1024/2;
  int *lbns = new int[n];
  int k=0;
  for(i=0;i<n;i++)
  {
    int lbn = rand()%(disk_size/page_size*1024);
    lbns[k] = lbn;
    //writeSSD(lbn,'1');
    if((i+1)%r==0)
    {
      int readi = rand()%(k);
      readSSD(lbns[readi]);
      if(i%100==0)
        starts = clock();
      if((i+1)%100==0)
      {
        ends = clock();
        printf("%d %d\n",i,ends-starts);
      }
      i++;
    }//if
     if(i%100==0)
        starts = clock();
      if((i+1)%100==0)
      {
        ends = clock();
        printf("%d %d\n",i,ends-starts);
      }
    k++;
  }//for
}

void SSD::sequenceTest(int n)
{
    int i;
    time_t starts,ends;
    for(i=0;i<n;i++)
    {
      //writeSSD(i,'1');
      if(i%100==0)
        starts = clock();
      if((i+1)%100==0)
      {
        ends = clock();
        printf("%d %d\n",i,ends-starts);
      }
    }
}

void SSD::printSTATE()
{
  myftl->printSTATE();
}
