#define VALID 1
#include <fcntl.h>  
#include <unistd.h>  
#define INVALID 2
#define FREE 0
#include "stdio.h"
#include "string.h"
class FTL
{
	public:
		FTL(int page_size):page_size(page_size),block_size(block_size)
		{
		    fp = open("data",O_RDWR | O_CREAT|O_SYNC,0700);
    }
    ~FTL()
    {
        printf("close\n");
        close(fp);
    }
		virtual int  writeFTL(int lbn,char *data){};
		virtual char *readFTL(int lbn){};
	protected:
		int block_size;
		int page_size;
		int *valid;
		virtual int findFreePBN(){};
		virtual int findPBN(int lbn){};
		int writePBN(int pbn,char *data);
		char *readPBN(int pbn);
		int fp;
	
};

class PFTL:public FTL
{
	public:
		int *p_map;
		PFTL(int page_num,int page_size):FTL(page_size),page_num(page_num)
		{

			//init p_map-->LBN->PBN
			p_map = new int[page_num];
			memset(p_map,-1,sizeof(int)*page_num);

			//init valid
	    valid = new int[page_num];
			memset(valid,FREE,sizeof(int)*page_num);
		}
		
	private:
		int page_num;
	protected:
		int findFreePBN();
		int findPBN(int lbn);
	  int writeFTL(int lbn,char*data);
    char* readFTL(int lbn);
};

class BFTL:public FTL
{
  	public:
		int *b_map;
		BFTL(long block_num,int block_size):FTL(block_size),block_num(block_num)
		{

			//init p_map-->LBN->PBN
			b_map = new int[block_num];
			memset(b_map,-1,sizeof(int)*block_num);

			//init valid
	    valid = new int[block_num];
			memset(valid,FREE,sizeof(int)*block_num);
		}
		
	
  private:
		int block_num;
};

