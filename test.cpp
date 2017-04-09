#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "mytool.h"
#define LEN 100000
int main(void)
{
  int num=0;
  int flag[LEN]={0};
  
  for(int j=0;j<LEN;j++)
  {
    flag[j] =0;
  }
  for(int i=0;i<100000;i++)
  {
    int lbn = rand()%LEN;
    //lbn = mymd5(lbn)%LEN;
    int k = lbn/64+lbn%64;
    if(flag[k]==0)
    {
      flag[k] = 1;
    }
    else
    {
      num++;
    }
  }
  printf("%d",num);
  return 1;

}
