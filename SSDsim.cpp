#include "SSDsim.h"


//init
SSD::SSD(int disk_size,int block_size, int page_size, int mem_size,int k_hash_num,int m_offset_num,int policy):
disk_size(disk_size),block_size(block_size),page_size(page_size),mem_size(mem_size),k_hash_num(k_hash_num),m_offset_num(m_offset_num),policy(policy)
{

	 long block_num = disk_size/block_size;
	 block_num = block_num<<GB2KB;

	 switch(policy)
	 {
		case PFTLNUM:
			disk_size = disk_size<<GB2KB;
			long page_num = disk_size/page_size;
			page_size = page_size<<KB2B;
			PFTL *ftl = new PFTL(page_num,page_size);
			myftl = ftl;
			
	 }
}
void SSD::writeSSD(int lbn)//data struct
{
	page_size = page_size<<KB2B;
  char *data = new char[page_size/sizeof(char)];
	memset(data,'1',page_size/sizeof(char));
	myftl->writeFTL(lbn,data);
}
char* SSD::readSSD(int lbn)
{
  char *data;
  data = myftl->readFTL(lbn);
  return data;
}

