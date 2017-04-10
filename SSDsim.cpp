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
   int dnum  = (mem_size - dftl_level)/8*1024;//first num;

   int hftl_level = (k_hash_num+m_offset_num)*page_num/32/1024;//first
   int hftl_len = (mem_size-hftl_level)/8*1024;

   printf("page:%d\nblock:%d\nDFTL cache num:%d\nHFTL table num:%d\n",pftl_level,bftl_level,dnum,hftl_len);
   
   page_size = page_size<<KB2B;
   block_size = block_size<<KB2B;
   
   switch(policy)
	 {			
		  case PFTLNUM:
        {
          PFTL *pftl = new PFTL(page_num,page_size);
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
          HFTL *hftl = new HFTL(block_num,block_size,page_num,page_size,mem_size,khn,mon);
          myftl = hftl;
        }
        break;
        
	 }
}
void SSD::writeSSD(int lbn,char ch)//data struct
{
  char *data = new char[page_size/sizeof(char)];
	memset(data,ch,page_size/sizeof(char));
	myftl->writeFTL(lbn,data);
}
char* SSD::readSSD(int lbn)
{
  char *data;
  data = myftl->readFTL(lbn);
  return data;
}

void SSD::randomTest(int n)
{
    int i;
    if(n==0)
      n = disk_size/page_size*1024/2;
    int *lbns = new int[n];
    time_t starts,ends;
    for(i=0;i<n;i++)
    {
      //srand(i);
      int lbn = rand()%(disk_size/page_size*1024);
      //lbn = mymd5(lbn)%(disk_size/page_size*1024);
      //int lbn = rand()%n;
      lbns[i] = lbn;
      if(i%100==0)
        starts = clock();
      writeSSD(lbn,'1');
      if((i+1)%100==0)
      {
        ends = clock();
        printf("%d %d\n",i,ends-starts);
      }
    }
    for(i=0;i<n;i++)
    {
      //readSSD(lbns[i]);
      //printf("%d:%s\n",lbns[i],readSSD(lbns[i]));
    }
    printf("random test ok\n");
}
void SSD::sequenceTest(int n)
{
    int i;
    for(i=0;i<n;i++)
    {
      writeSSD(i,'1');
    }
}

void SSD::printSTATE()
{
  myftl->printSTATE();
}
