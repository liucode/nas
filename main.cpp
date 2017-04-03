#include "SSDsim.h"
int main(void)
{
    SSD *ssd = new SSD(1,128,4,512,4,6,2);
    ssd->sequenceTest(10);
    delete ssd;
    return 1;
}
