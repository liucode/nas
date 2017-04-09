#include "malloc.h"
#include "stdlib.h"
#include "string.h"
#include "openssl/md5.h"
#define HASHLEN 1000
typedef struct LRUnode
{
  struct LRUnode *next;
  struct LRUnode *pre;
  int lbn;
  int pbn;
  struct LRUnode *hashnext;
  struct LRUnode *hashpre;
}*lnode;

typedef struct LRUlist
{
  struct LRUnode *head;
  int len;
  struct LRUnode *tail;
}*llist;


typedef struct HASHnode
{
    int pbn;
    int lbn;
    struct HASHnode *next;
}*hnode;

lnode LRUread(llist list,lnode p,int ms);
// true: NULL delete:lnode
lnode LRUinsert(llist list,lnode p,int ms);
lnode LRUfind(llist list,int lbn);
int LRUdelete(llist list,int lbn);

int HSinsert(hnode *list,int lbn,int pbn,int hashlen);
hnode HSfind(hnode *list,int lbn,int hashlen);

int NMinsert(llist list,int lbn,int pbn);
lnode NMfind(llist list,int lbn);

lnode HSLinsert(lnode *ht,llist lt,int lbn,int pbn,int ms);
lnode HSLread(lnode *ht,llist lt,int lbn,int ms);


int mymd5(int input);

