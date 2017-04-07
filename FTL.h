#define VALID 1
#include <fcntl.h>  
#include <unistd.h>  
#include "math.h"
#define INVALID 2
#define FREE 0
#include "stdio.h"
#include "string.h"
#define LBNLEN 512
#define DATALEN 512
#include <stdlib.h>
#include "mytool.h"
#define LOG 4
#define DEBUG 0
class FTL
{
	public:
		FTL(int page_size,int block_size):page_size(page_size),block_size(block_size)
		{
		    fp = open("data",O_RDWR | O_CREAT|O_DIRECT,0700);
    }
    ~FTL()
    {
        printf("close\n");
        close(fp);
    }
		virtual int  writeFTL(int lbn,char *data){};
    void printSTATE();
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


    //statistics
    int movenum =0;
    int logwritenum = 0;
    int cachenum =0;
    int pagewritenum = 0;
    int overwritenum = 0;
	  int tblocknum = 0;
    
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
      logfp = open("log",O_RDWR|O_CREAT|O_DIRECT|O_APPEND,0700);

      per_page = page_num/block_num;
   	  
      //init OOB
      OOB = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
 
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
    int *OOB;
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
		DFTL(int block_num,int block_size,int page_num,int page_size,int ms):FTL(page_size,block_size),block_num(block_num),page_num(page_num),ms(ms)
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
   	   
      //init translation block valid
      tb_valid = new int[page_num];
			memset(tb_valid,FREE,sizeof(int)*page_num);
   	  
      //init OOB
      OOB = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
   		
      //init cmt
      cmt = (llist) malloc (sizeof(struct LRUlist));
      cmt->len = 0;
      cmt->head = NULL;
    
      //init trans file
      tfp = open("tblock",O_RDWR|O_CREAT|O_DIRECT,0700);
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
    int *tb_valid;
    int per_page;
    int *OOB;
    int ms;
    protected:
		int findFreePagePBN();
    int findFreeBlockPBN();
		int findPBN(int lbn);
	  int writeFTL(int lbn,char*data);
    char* readFTL(int lbn);
    void liupwrite(int fp,int data,int len,int offset);
};


class HFTL:public FTL
{
  	public:
		HFTL(int block_num,int block_size,int page_num,int page_size,int ms,int khn,int mon):FTL(page_size,block_size),block_num(block_num),page_num(page_num),khn(khn),mon(mon)
		{
			//init p_map-->LBN->PBN Global Translation Directory
			p_map = new int[page_num];
			memset(p_map,-1,sizeof(int)*page_num);

			//init valid :get it used hash()
	    valid = new int[block_num];
			memset(valid,FREE,sizeof(int)*block_num);

      //init page valid
      p_valid = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
   	   
   	  
      //init OOB
      OOB = new int[page_num];
			memset(p_valid,FREE,sizeof(int)*page_num);
   		
      per_page = page_num/block_num;
      
      //init cmt
      cmt = new hnode[hashlen];
      for(int i=0;i<hashlen;i++)
      {
        cmt[i] = NULL;
      }

    }
    ~HFTL()
    {
      free(cmt);
    }
    private:
    int hashlen = 100;
    
    hnode *cmt;

    int khn;// n binary
    int mon;


    int block_num;
    int page_num;
    
    int tfp;
    
    int *p_valid;
    
    int per_page;
    int *OOB;
    int *p_map;

    int truepbn;
    
    protected:
		int findFreePBN(int lbn);
    int findTruePBN(int lbn,int pbn);
		int findPBN(int lbn);
	  int writeFTL(int lbn,char*data);
    char* readFTL(int lbn);
};
