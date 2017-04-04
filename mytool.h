#include "malloc.h"
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

lnode LRUread(llist list,int lbn);
// true: NULL delete:lnode
lnode LRUinsert(llist list,int lbn,int pbn,int ms);
lnode LRUfind(llist list,int lbn);
int LRUdelete(llist list,int lbn);
