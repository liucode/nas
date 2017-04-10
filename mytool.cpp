#include "mytool.h"
int mymd5(int input)
{
    int i;
    unsigned char  d[16];
    char *out = NULL;
    char password[32];
    sprintf (password,"%d",input);
    MD5_CTX x;
    MD5_Init(&x);
    MD5_Update(&x,(char*)password,strlen(password));
    MD5_Final(d,&x);
    out = new char[9];
    memset (out, 0x00, 9);
    for  (i = 0; i < 9; i++)
    {
      sprintf (out+i, "%d" , d[i]);  // 转换为32位
    }
    int m = strtoul(out, NULL, 0);
    if(m<0)
      m = 0-m;
    return m;
}

lnode LRUread(llist list,lnode p,int ms)
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
            list->head = p->next;
            p->next->pre = p->pre;
        }
        else
        {
            list->head = NULL;
            list->tail = NULL;
        }
        list->len--;
        //insert
        LRUinsert(list,p,ms);
        return NULL;
}


// true: NULL delete:lnode
lnode LRUinsert(llist list,lnode p,int ms)
{
      //if(list->tail!=NULL)
      //printf("tail: %d -->%d\n",list->tail->lbn,list->tail->pbn);
      lnode q = NULL;
      if(list->head == NULL)
      {
        p->pre =NULL;
        p->next = NULL;
        list->head = p;
        list->tail = p;
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



int NMinsert(llist list,int lbn,int pbn)
{
    lnode p = list->tail;
    lnode q = (lnode)malloc(sizeof(struct LRUnode));
    lnode k = NMfind(list,lbn);
    if(k!=NULL)
    {
      k->pbn = pbn;
      return 1;
    }
    else
    {
      if(list->head==NULL)
      {
        q->next = NULL;
        q->pre = NULL;
        list->head = q;
        list->tail = q;
      }
      else
        p->next = q;
    
      q->lbn = lbn;
      q->pbn = pbn;
      q->pre = p;
      q->next = NULL;
      list->tail = q;
    }
    return 0;
}
lnode NMfind(llist list,int lbn)
{
    lnode p = list->head;
    while(p)
    {
      if(lbn == p->lbn)
      {
        return p;
      }
      p = p->next;
    }
    return NULL;
}


//HASH
int HSinsert(hnode *list,int lbn,int pbn,int hashlen)
{
    int k = lbn%hashlen;
    if(list[k] == NULL)
    {
      hnode q = (hnode)malloc(sizeof(struct HASHnode));
      q->lbn = lbn;
      q->pbn = pbn;
      q->next = NULL;
      list[k] = q;
    }
    else
    {
      hnode p;
      p = list[k];
      while(p->next)
      {
          if(p->lbn == lbn)
          {
            p->pbn = pbn;
            return 1;
          }
          p = p->next;
      }
      if(p->lbn == lbn)
      {
        p->pbn = pbn;
        return 1;
      }
      hnode q = (hnode)malloc(sizeof(struct HASHnode));
      q->lbn = lbn;
      q->pbn = pbn;
      q->next = NULL;
      p->next =q;
    }
    return 0;
}
hnode HSfind(hnode *list,int lbn,int hashlen)
{
      int k=lbn%hashlen;
      hnode p;
      p = list[k];
      while(p->next)
      {
          if(p->lbn == lbn)
          {
            return p;
          }
          p = p->next;
      }
      if(p->lbn == lbn)
      {
        return p;
      }
      return NULL;
 
}


//HASH+LRU
lnode HSLinsert(lnode *ht,llist lt,int lbn,int pbn,int ms)
{
    int k = lbn%HASHLEN;
    lnode l=NULL;
    if(ht[k] == NULL)
    {
      lnode q = (lnode)malloc(sizeof(struct LRUnode));
      q->lbn = lbn;
      q->pbn = pbn;
      q->hashnext = NULL;
      q->hashpre = NULL;
      ht[k] = q;
      l = LRUinsert(lt,q,ms);
    }
    else
    {
      lnode p;
      p = ht[k];
      while(p->hashnext)
      {
          if(p->lbn == lbn)
          {
            p->pbn = pbn;
            LRUread(lt,p,ms);
            return NULL;
          }
          p = p->hashnext;
      }
      if(p->lbn == lbn)
      {
        p->pbn = pbn;
        LRUread(lt,p,ms);
        return p;
      }
      lnode q = (lnode)malloc(sizeof(struct LRUnode));
      q->lbn = lbn;
      q->pbn = pbn;
      q->hashnext = NULL;
      p->hashnext =q;
      q->hashpre = p;
      l = LRUinsert(lt,q,ms);
    }
    if(l!=NULL)
    {
        if(l->hashpre !=NULL&&l->hashnext!=NULL)
        {
          l->hashpre->hashnext = l->hashnext;
          l->hashnext->hashpre = l->hashpre;
        }
        else if(l->hashpre !=NULL&&l->hashnext ==NULL)
        {
           l->hashpre->hashnext = l->hashnext;//NULL
        }
        else if(l->hashnext !=NULL&&l->hashpre == NULL)
        {
            ht[(l->lbn)%HASHLEN] = l->hashnext;
            ht[(l->lbn)%HASHLEN]->hashpre = NULL;
        }
        else
        {
          ht[(l->lbn)%HASHLEN] =  NULL;
        }
     
    }
    return l;
}

lnode HSLread(lnode *ht,llist lt,int lbn,int ms)
{
      int k=lbn%HASHLEN;
      lnode p;
      p = ht[k];
      while(p)
      {
          if(p->lbn == lbn)
          {
            LRUread(lt,p,ms);
            return p;
          }
          p = p->hashnext;
      }
      return NULL;
}

int HSLprint(llist lt)
{
  lnode p = lt->head;
  if(p)
  {
    while(p->next)
  {
    p = p->next;
  }
  if(lt->tail->lbn!=p->lbn)
  {
    return 0;
  }
  else
  {
    return 1;
  }
  }
  return 1;
}

