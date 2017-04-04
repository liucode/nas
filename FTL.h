#define VALID 1
#include <fcntl.h>  
#include <unistd.h>  
#include "tool.h"
#define INVALID 2
#define FREE 0
#include "stdio.h"
#include "string.h"
#define LBNLEN 32
# include <stdlib.h>
class FTL
{
	public:
		FTL(int page_size,int block_size):page_size(page_size),block_size(block_size)
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
		PFTL(int page_num,int page_size):FTL(page_size,0),page_num(page_num)
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
		BFTL(int block_num,int block_size,int page_num,int page_size):FTL(page_size,block_size),block_num(block_num),page_num(page_num)
		{

			//init p_map-->LBN->PBN
			b_map = new int[block_num];
			memset(b_map,-1,sizeof(int)*block_num);

			//init valid
	    valid = new int[block_num];
			memset(valid,FREE,sizeof(int)*block_num);

      //init page valid
      p_valid = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);

     

      //log block
      logfp = open("log",O_RDWR | O_CREAT|O_SYNC|O_APPEND,0700);

      per_page = page_num/block_num;

		}
    ~BFTL()
    {
      printf("close log file\n");
      close(logfp);
    }
		
	
  private:
    int logfp;
		int block_num;
    int page_num;
    int per_page;
    int *p_valid;
 	protected:
		int findFreePBN();
		int findPBN(int lbn);
	  int writeFTL(int lbn,char*data);
    char* readFTL(int lbn);
    int movePBN(int lbn,char *data);
    int writeLOG(int lbn,char *data);
};


class DFTL:public FTL
{
  	public:
		DFTL(int block_num,int block_size,int page_num,int page_size):FTL(page_size,block_size),block_num(block_num),page_num(page_num)
		{

			//init p_map-->LBN->PBN Global Translation Directory
			b_map = new int[block_num];
			memset(b_map,-1,sizeof(int)*block_num);

			//init valid
	    valid = new int[block_num];
			memset(valid,FREE,sizeof(int)*block_num);

      //init page valid
      p_valid = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
   	  OBB = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
   		
      //init cmt
      cmt = (llist) malloc (sizeof(struct LRUlist));
      cmt->len = 0;
      cmt->head = NULL;
    
      //init trans file
      tfp = open("tblock",O_RDWR | O_CREAT|O_SYNC|O_APPEND,0700);
      per_page = page_num/block_num;

    }
    ~DFTL()
    {
      free(cmt);
      close(tfp);
    }
    private:
    int block_num;
    int page_num;
    int tfp;
    llist cmt;//Cached Mapping Table
    int *b_map;//Global Translation Directory
    int *p_valid;
    int per_page;
    int *OBB;
    protected:
		int findFreePagePBN();
    int findFreeBlockPBN();
		int findPBN(int lbn);
	  int writeFTL(int lbn,char*data);
    char* readFTL(int lbn);
};
	
