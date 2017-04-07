#include "malloc.h"
#include "stdlib.h"
#include "string.h"
#include "openssl/md5.h"
typedef struct LRUnode
{
  struct LRUnode *next;
  struct LRUnode *pre;
  int lbn;
  int pbn;
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

lnode LRUread(llist list,int lbn,int ms);
// true: NULL delete:lnode
lnode LRUinsert(llist list,int lbn,int pbn,int ms);
lnode LRUfind(llist list,int lbn);
int LRUdelete(llist list,int lbn);

int HSinsert(hnode *list,int lbn,int pbn,int hashlen);
hnode HSfind(hnode *list,int lbn,int hashlen);

int NMinsert(llist list,int lbn,int pbn);
lnode NMfind(llist list,int lbn);
int mymd5(int input);

