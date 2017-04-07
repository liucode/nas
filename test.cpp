#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
int main(void)
{
	int fd = open("testdata",O_RDWR | O_CREAT|O_DIRECT,0700);
  char  buf[2048] = "1234567890";
	void *align_buf = NULL; 
/* 假设 /sys/block/sda/queue/logical_block_size 为 512B */
if (posix_memalign(&align_buf, 512, sizeof(buf)) != 0) {
    perror("memalign failed");
    return 1;
}
int offset = 0;
int len = pwrite(fd, align_buf, sizeof(buf), offset);
printf("%d\n",len);
return 1;
}
