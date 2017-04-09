#include "SSDsim.h"
#include "time.h"
int main(int argc, char *argv[])
{
  //disk_size(ds<<GB2B),block_size(bs<<KB2B),page_size(ps<<KB2B),mem_size(ms<<KB2B:),k_hash_num(khn),m_offset_num(mon),policy(p)
    int policy = atoi(argv[1]);
    time_t starts,ends;  
    //int policy = 3;
    int disk_size = 2;
    int block_size =128;
    int page_size = 4;
    int mem_size = 300;
    int k_hash_num = 4;
    int m_offset_num =4;
    SSD *ssd = new SSD(disk_size,block_size,page_size,mem_size,k_hash_num,m_offset_num,policy);
    starts = clock();
    ssd->randomTest(3000);
    ends = clock();
    printf("time:%d\n",ends-starts);
    //ssd->sequenceTest(100000);
    ssd->printSTATE();
    delete ssd;
    return 1;
}
