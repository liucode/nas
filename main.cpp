#include "SSDsim.h"
int main(void)
{
    SSD *ssd = new SSD(1,128,4,512,4,6,1);
    ssd->writeSSD(1);
    char *data;
    data = ssd->readSSD(1);
    printf("data:%s\n",data);
    delete ssd;
    return 1;
}
