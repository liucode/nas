#include "mytool.h"

lnode LRUread(llist list,int lbn)
{
    lnode p = list->head;
    while(p)
    {
      if(p->lbn == lbn)
      {
        int pbn = p->pbn;
        //del
        p->pre->next = p->next;
        p->next->pre = p->pre;
        
        //insert
        p->next = list->head;
        list->head->pre = p;
        p->pre = NULL;
        list->head = p;
        return p;
      }
      p = p->next;
    }
    return NULL;
}


// true: NULL delete:lnode
lnode LRUinsert(llist list,int lbn,int pbn,int ms)
{
      lnode p=(lnode)malloc(sizeof(struct LRUnode));
      lnode q = NULL;
      p->lbn = lbn;
      p->pbn = pbn;

      if(list->head =NULL)
      {
        list->head = p;
        list->head->pre = NULL;
        list->head->next = NULL;
      }
      
      p->next = list->head;
      list->head->pre = p;
      list->head = p;
      list->head->pre = NULL;
      if(ms>list->len+1)
      {
        list->len++;
      }
      else
      {
        q = list->tail;
        list->tail->pre->next = NULL;
        list->tail = list->tail->pre;
      }
      return q;
}


lnode LRUfind(llist list,int lbn)
{
    lnode p = list->head;
    while(p)
    {
      if(p->lbn == lbn)
      {
        return p;
      }
      p = p->next;
    }
    return NULL;
  
}

int LRUdelete(llist list,int lbn)
{
    lnode p = list->head;
    while(p)
    {
      if(p->lbn == lbn)
      {
        int pbn = p->pbn;
        //del
        p->pre->next = p->next;
        p->next->pre = p->pre;
        return pbn;
      }
      p = p->next;
    }
    return -1;
}



