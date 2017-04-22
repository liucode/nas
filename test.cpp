#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>
#include <memory.h>
#include "fcntl.h"
#define DATALEN 512

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


int main(void)
{
  	int fp = open("/dev/sdc",O_RDWR|O_CREAT|O_DIRECT,0700);
		char *data = new char[4096];
    memset(data,'5',4096);
    void *align_buf = NULL;
    if (posix_memalign(&align_buf,DATALEN,4096) != 0)  
    {   
        printf("memalign failed\n");
    }   
    strcpy((char *)align_buf,data);
	double s,e;
	s = debug_time_sec();  
	char * td = (char *)align_buf;
  for(int i=0;i<500000;i++)
	{
		printf("%d\n",i);
		int offset =i*4096;
    if(pwrite(fp,td,4096,offset)!=4096)
  	{
    	printf("error writePBN\n");
    	exit(0);
  	}
	}
	
	e = debug_time_sec();
	close(fp);
  printf("%f",e-s);
  return 0;
}
