#include "SSDsim.h"
#include "time.h"
int main(int argc, char *argv[])
{
  //disk_size(ds<<GB2B),block_size(bs<<KB2B),page_size(ps<<KB2B),mem_size(ms<<KB2B:),k_hash_num(khn),m_offset_num(mon),policy(p)
    int policy = atoi(argv[1]);
    //int policy = 3;
    int disk_size = atoi(argv[5]);
    int block_size = 128;
    int page_size = 4;
    int mem_size = disk_size*atoi(argv[2]);
    //int k_hash_num = 5;
    int k_hash_num = atoi(argv[3]);
    int m_offset_num = atoi(argv[4]);
    SSD *ssd = new SSD(disk_size,block_size,page_size,mem_size,k_hash_num,m_offset_num,policy);
    int sle = atoi(argv[6]);
    if(sle == 1)
      ssd->zfTest(16000000);
    else if(sle == 2)
      ssd->randomTest(16000000);
    else
      ssd->fileTest(1000000,argv[7]);
    //ssd->srTest(4,100000);
    //ssd->sequenceTest(100000);
    ssd->printSTATE();
    delete ssd;
    return 1;
}
