#include "SSDsim.h"
//init
SSD::SSD(int ds,int bs, int ps, int ms,int khn,int mon,int p):
disk_size(ds<<GB2B),block_size(bs<<KB2B),page_size(ps<<KB2B),mem_size(ms),k_hash_num(khn),m_offset_num(mon),policy(p)
{

	 int block_num = disk_size/block_size;
	 int page_num = disk_size/page_size;
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
          BFTL *bftl = new BFTL(block_num,block_size);
          myftl = bftl;
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
    int *lbns = new int[n];
    for(i=0;i<n;i++)
    {
      int lbn = rand()%(disk_size/page_size);
      lbns[i] = lbn;
      writeSSD(lbn,'1');
    }
    for(i=0;i<n;i++)
    {
      readSSD(lbn[i]);
      //printf("%d:%s\n",lbns[i],readSSD(lbns[i]));
    }
    printf("random test ok\n");
}
