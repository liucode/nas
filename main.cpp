#include "SSDsim.h"
#include "stdlib.h"
#include "time.h"
#include "md5.h"
int main(int argc, char *argv[])
{
    //disk_size(ds<<GB2B),block_size(bs<<KB2B),page_size(ps<<KB2B),mem_size(ms),k_hash_num(khn),m_offset_num(mon),policy(p)
    int policy = atoi(argv[1]);
    time_t starts,ends;  
    //int policy =4;
    SSD *ssd = new SSD(1,128,4,250,4,4,policy);
    starts = clock();
    ssd->randomTest(50);
    md5('a');
    ends = clock();
    printf("time:%d\n",ends-starts);
    //ssd->sequenceTest(10);
    ssd->printSTATE();
    delete ssd;
    return 1;
}
