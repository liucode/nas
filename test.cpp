#include <stdio.h>
#include <stdlib.h>
#define LEN 100000
int main(void)
{
  
  for(int i=0;i<100000;i++)
  {
    int h = i/10+1;
    int lbn = rand()%h;
  }
  return 1;

}
