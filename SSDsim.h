#include <stdio.h>
#include "FTL.h"
#include <stdlib.h>
#define GB2MB 10
#define GB2KB 20
#define MB2KB 10
#define GB2B 30
#define KB2B 10
#define PFTLNUM 1
#define BFTLNUM 2

class SSD{
	public:
		SSD(int disk_size,int block_size, int page_size, int mem_size,int k_hash_num,int m_offset_num,int policy);
		void writeSSD(int lbn,char ch);
    char* readSSD(int lbn);
    void randomTest(int n);
    ~SSD()
    {
      delete myftl;
    }
	private:
		int disk_size;// GB
		int block_size;//KB
		int page_size;//KB
		int mem_size;//MB

		int k_hash_num;//number of hash fun

		int m_offset_num;
		
		int policy;
		
		FTL *myftl;
};
