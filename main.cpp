#include "SSDsim.h"
int main(void)
{
    SSD *ssd = new SSD(1,128,4,5,4,6,3);
    ssd->randomTest(1000);
    //ssd->sequenceTest(10);
    ssd->printSTATE();
    delete ssd;
    return 1;
}
