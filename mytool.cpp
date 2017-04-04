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
        if(p->pre != NULL&&p->next !=NULL)//at last
        {
          p->pre->next = p->next;
          p->next->pre = p->pre;
        }
        else if(p->pre !=NULL&&p->next == NULL)
        {
          list->tail = p->pre;
          p->pre->next = p->next;
        }
        else if(p->next!=NULL&&p->pre == NULL)
        {
          p->next->pre = p->pre;
        }
        else
        {
            list->head = NULL;
            list->tail = NULL;
        }
        //insert
        p->next = list->head;
        list->head->pre = p;
        p->pre = NULL;
        list->head = p;
        if(list->head->next == NULL)
          list->tail = p;
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

      if(list->head == NULL)
      {
        list->head = p;
        list->tail = p;
        list->head->pre = NULL;
        list->head->next = NULL;
        return NULL;
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



