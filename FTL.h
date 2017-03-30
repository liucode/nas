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
		FTL(int page_size):page_size(page_size)
		{
		    fp = open("data",O_RDWR | O_CREAT|O_SYNC,0700);
    }
    ~FTL()
    {
        printf("close\n");
        close(fp);
    }
		int  writeFTL(int lbn,char *data);
		char *readFTL(int lbn);
	protected:
		int block_size;
		int page_size;
		int *valid;
		virtual int findFreePBN();
		virtual int findPBN(int lbn);
		int writePBN(int pbn,char *data);
		char *readPBN(int pbn);
		int fp;
	
};

class PFTL:public FTL
{
	public:
		int *p_map;
		PFTL(long page_num,int page_size):FTL(page_size),page_num(page_num)
		{

			//init p_map-->LBN->PBN
			p_map = new int[page_num];
			memset(p_map,0,page_num);

			//init valid
	    valid = new int[page_num];
			memset(valid,FREE,page_num);
		}
		
	private:
		long page_num;
	protected:
		int findFreePBN();
		int findPBN(int lbn);
	
};

class BFTL
{
	private:
		long block_num;
};

