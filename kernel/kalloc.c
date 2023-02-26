// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "sleeplock.h"

#define MAX_LIVE_PAGE 30

void kfree2(void *pa);
void removeLast();
void swapListSize();

int debug = 0;

struct sleeplock slock;
struct run {
  struct run *next;
};
struct {
  char count[PAGE_COUNT];
  struct spinlock lock;
} refCount;


struct liveListNode {
  pte_t *pte;
  int procId;
  int vpn;
  struct liveListNode* next;
};

struct swappedListNode{
  int procId;
  int vpn;
  struct swap* sp;
  struct swappedListNode* next;
  pte_t *pte;
};


struct {
  struct spinlock lock;
  struct run *freelist;
} liveListNodeMem, swappedListNodeMem;

void
liveListNodeInit(void)
{
  initlock(&liveListNodeMem.lock, "liveListNodeMem");
  liveListNodeMem.freelist = 0;
}

struct liveListNode *
liveListNodeAlloc(void)
{
  struct run *r;
  struct liveListNode *s;

  acquire(&liveListNodeMem.lock);
  r = liveListNodeMem.freelist;
  if(!r){
    release(&liveListNodeMem.lock);
    char *mem = kalloc();
    char *mem_end = mem + PGSIZE;
    for(; mem + sizeof(struct liveListNode) <= mem_end; mem += sizeof(struct liveListNode)){
      r = (struct run*)mem;

      acquire(&liveListNodeMem.lock);
      r->next = liveListNodeMem.freelist;
      liveListNodeMem.freelist = r;
      release(&liveListNodeMem.lock);
    }
    acquire(&liveListNodeMem.lock);
    r = liveListNodeMem.freelist;
  }
  liveListNodeMem.freelist = r->next;
  release(&liveListNodeMem.lock);
  
  s = (struct liveListNode*)r;

  
  return s;
}
void
liveListNodeFree(struct liveListNode *s)
{
  struct run *r;

  if(!s)
    panic("swapfree");
  s->next = 0;
  s->procId = 0;
  s->pte = 0;
  s->vpn = 0;
  r = (struct run*)s;
  acquire(&liveListNodeMem.lock);
  r->next = liveListNodeMem.freelist;
  liveListNodeMem.freelist = r;
  release(&liveListNodeMem.lock);
}

void
swappedListNodeInit(void)
{
  initlock(&swappedListNodeMem.lock, "swappedListNodeMem");
  swappedListNodeMem.freelist = 0;
}

struct swappedListNode *
swappedListNodeAlloc(void)
{
  struct run *r;
  struct swappedListNode *s;

  acquire(&swappedListNodeMem.lock);
  r = swappedListNodeMem.freelist;
  if(!r){
    release(&swappedListNodeMem.lock);
    char *mem = kalloc();
    char *mem_end = mem + PGSIZE;
    for(; mem + sizeof(struct swappedListNode) <= mem_end; mem += sizeof(struct swappedListNode)){
      r = (struct run*)mem;

      acquire(&swappedListNodeMem.lock);
      r->next = swappedListNodeMem.freelist;
      swappedListNodeMem.freelist = r;
      release(&swappedListNodeMem.lock);
    }
    acquire(&swappedListNodeMem.lock);
    r = swappedListNodeMem.freelist;
  }
  swappedListNodeMem.freelist = r->next;
  release(&swappedListNodeMem.lock);
  
  s = (struct swappedListNode*)r;

  
  return s;
}
void
swappedListNodeFree(struct swappedListNode *s)
{
  struct run *r;

  if(!s)
    panic("swapfree");
  s->next = 0;
  s->procId = 0;
  s->vpn = 0;
  s->sp = 0;
  r = (struct run*)s;
  acquire(&swappedListNodeMem.lock);
  r->next = swappedListNodeMem.freelist;
  swappedListNodeMem.freelist = r;
  release(&swappedListNodeMem.lock);
}


struct {
  struct liveListNode* list;
  // struct spinlock lock;
  // struct sleeplock slock;
  int count[PAGE_COUNT];
  int liveCount;
} live;

struct {
  struct swappedListNode* list;
  // struct spinlock lock;
  // struct sleeplock slock;
} swapped;

void liveListInit(){
  // initlock(&live.lock, "livelock");
  // initsleeplock(&live.slock, "liveSleepLock");

  for(int i = 0; i < PAGE_COUNT; ++i){
    live.count[i] = 0;
  }
  live.list = liveListNodeAlloc();
 
  if(live.list == 0){
    panic("live head empty");
  }
  live.liveCount = 0;
  live.list->next = 0;
  live.list->pte = 0;
  live.list->procId = 0;
  live.list->vpn = 0;
}
void swappedListInit(){
  // initlock(&swapped.lock, "swappedlock");
  // initsleeplock(&swapped.slock, "swappedSleepLock");
  swapped.list = swappedListNodeAlloc();
  if(swapped.list == 0){
    panic("swapped list init");
  }
  else{
    swapped.list->next = 0;
    swapped.list->procId = 0;
    swapped.list->sp = 0;
    swapped.list->vpn = 0;
    swapped.list->pte = 0;
  }
}
void swapOut(struct liveListNode* n){
  if(debug)
    printf("swapOut en\n");
  if(n == 0){
    panic("null pointer to swap");
  }
  struct swap* s = swapalloc();
  if(s == 0){
    panic("swap not allocated");
  }
  if(*n->pte & PTE_SWAPPED){
    panic("swapped bit on __");
  }
  // printf("swapout en\n");
      // // releasesleep(&slock);
  swapout(s,(char*) PTE2PA(*n->pte));
      // // acquiresleep(&slock);
  // printf("swapout ex\n");
  
  int rc = 0;

  int f = 0;
  int removedPpn = PTE2PPN(*n->pte);
  // acquire(&live.lock);
  // // acquiresleep(&live.slock);
  struct liveListNode* l = live.list;
  if(l == 0){
    panic("live list head empty\n");
  }
  // struct liveListNode* found;
  while(l->next){
    
    int lppn = PTE2PPN(*l->next->pte);
    if(removedPpn == lppn){
    
      // if(l->next->procId == 3 && l->next->vpn == 0){
      //   printf("--------\n\n");
      // }
      // printf("========\n");
      f = 1;
      *l->next->pte &= (~PTE_V);
      *l->next->pte |= PTE_SWAPPED;
      struct liveListNode* curr = l->next;
      
      struct swappedListNode* sn = 0;

      sn = swappedListNodeAlloc();
      if(sn == 0){
        panic("swapped list node 0\n");
      }
      sn->next = 0;
      sn->procId = curr->procId;
      sn->vpn = curr->vpn;
      // printf("sn->pid %d\n", sn->procId);
      sn->sp = s;
      sn->pte = curr->pte;
      // release(&live.lock);
      if(swapped.list == 0){
        panic("swapped list head 0");
      }
      sn->next = swapped.list->next;
      swapped.list->next = sn;
      // if(sn->procId == 3 && sn->vpn == 0){
      //   printf("\n\nswapped\n\n");
      // }
      // swapListSize();
      // acquire(&live.lock);
      struct liveListNode* t = l->next->next;
      liveListNodeFree(l->next);
      l->next = t;
      ++rc;
    }
    else{
      l = l->next;
    }
  }
  if(!f){
    panic("pte not asdffound\n");
  }
  
  // if(PTE2PPN(l->pte) == removedPpn){
  //   *l->pte &= (~PTE_V);
  //   *l->pte |= PTE_SWAPPED;
  //   liveListNodeFree(l);
  //   live.list = 0;
  // }
  if(live.count[removedPpn] != rc){
    panic("rc not m");
  }
  live.count[removedPpn] = 0;
  live.liveCount--;
  
  // printf("rc: %d s: %p\n", rc, s);
  swapSetRefCount(s, rc);
  // // releasesleep(&live.slock);

  // acquire(&swapped.lock);

  // release(&swapped.lock);
  

  // release(&live.lock);
  
  
  acquire(&refCount.lock);
  refCount.count[removedPpn] = 0;
  release(&refCount.lock);
  kfree2((void*) PPN2PA(removedPpn));
  // issue
  if(debug)
    printf("swapOut ex\n");
}

void swapListSize(){
  // // acquiresleep(&slock);
  // acquire(&swapped.lock);
  struct swappedListNode* s;
  s = swapped.list;
  int c = 0;
  printf("=============swap list stats============\n");
  while(s->next){
    if(debug)
      printf("pid->%d vpn->%d ppn:%d sp:%p\n", s->next->procId, s->next->vpn, PTE2PPN(*s->next->pte), s->next->sp);
    ++c;
    s = s->next;
  }
  printf("swapped list size: %d\n", c);
  // // releasesleep(&slock);
  // release(&swapped.lock);
}

void addSwapped(pte_t *pte, int oldProcId, int newProcId, int vpn){
  if(debug)
    printf("addswap en\n");
  if(*pte & PTE_V){
    panic("valid _");
  }
  // acquiresleep(&slock);
  // printf("%d %d %d\n",oldProcId, newProcId, vpn);
  struct swappedListNode* new = swappedListNodeAlloc();
  if(new == 0){
    panic("swapped node alloc");
  }
  new->next = 0;
  new->procId = newProcId;
  new->pte = pte;
  new->sp = 0;
  new->vpn = vpn;

  // acquire(&swapped.lock);
  if(swapped.list == 0){
    panic("swapped list head null");
  } 
  struct swappedListNode* curr = swapped.list;
  int f = 0;
  while(curr->next){
    if(curr->next->procId == oldProcId && curr->next->vpn == vpn){
      new->sp = curr->next->sp;
      swapIncCount(new->sp);
      f = 1;
      break;
    }
    curr = curr->next;
  }
  if(!f){
    printf("oPid: %d nPid:%d vpn:%d\n", oldProcId, newProcId, vpn);
    getLiveCount();
    swapListSize();
    panic("addswap: swap not found");
  }
  new->next = swapped.list->next;
  swapped.list->next = new;
  // release(&swapped.lock);
  if(debug)
    printf("addswap ex\n");
  // releasesleep(&slock);
}

void swapIn(int vpn, int procId, uint64 *pte){
  if(debug)
    printf("vpn:%d pid:%d valid:%d\n", vpn, procId, (*pte & PTE_V));
  // acquiresleep(&slock);

  // release space
  if(debug)
    printf("swapIn en\n");
  // printf("vpn %d procId %d\n", vpn, procId);
  // acquire(&live.lock);
  while(live.liveCount >= MAX_LIVE_PAGE){
    // release(&live.lock);
    removeLast();
    // acquire(&live.lock);
  }
  if(*pte & PTE_V){
    printf("vpn: %d pId %d\n",  vpn, procId);
    getLiveCount();
    swapListSize();
    panic("valid\n");
    // // printf("here\n")
    // return ;
  }
  // release(&live.lock);
  // swapListSize();
  struct swappedListNode* n;
  // acquire(&swapped.lock);
  n = swapped.list;
  if(n == 0){
    panic("swap list empty\n");
  }
  int f = 0;
  struct swap* s;
  char* mem;
  int rfc = 0;
  while(n->next){
    // printf("%d %d %d\n",procId, n->next->procId, n->next->vpn);
    if(n->next->procId == procId && n->next->vpn == vpn){
      
      f = 1;
      // release(&swapped.lock);
      mem =(char *) kalloc();
      // printf("%d\n",swapGetCount(n->next->sp));
      // printf("swapin en\n");
      if(n->next->sp == 0){
        panic("sp 0\n");
      }
      // // releasesleep(&slock);
      swapin(mem,n->next->sp);
      // // acquiresleep(&slock);
      // printf("swapin ex\n");
      s = n->next->sp;
      break;
    }
    else{
      n = n->next;
    }
  }
  if(!f){
    printf("looking for pid : %d vpn : %d\n", procId, vpn);
    getLiveCount();
    swapListSize();
    panic("swap in: swap not found\n");
  }
  // acquire(&swapped.lock);

  n = swapped.list;
  while(n->next){
    if(n->next->sp == s){
      // if(n->next->procId == 3 && n->next->vpn == 0){

      //   printf("\nremoved by prodId %d vpn %d\n\n", procId, vpn);
      // }
      // if(n->next->procId == 2 && n->next->vpn == 4){
      //   // printf("________found_________\n");
      // }
      if(s == 0){
        panic("none should be matched");
      }
      // printf("nxt pid: %d vpn : %d sp: %p s: %p\n", n->next->procId, n->next->vpn, n->next->sp, s);

      pte_t *pte = n->next->pte;
      // printf("cow flag1: %d\n", *pte & PTE_COW);
      *pte = (PTE_FLAGS(*pte)) | (PA2PTE((uint64)mem)) | (PTE_V);
      *pte &= (~PTE_SWAPPED);
      // printf("cow flag2: %d\n", *pte & PTE_COW);

      int pid = n->next->procId;
      int vp = n->next->vpn;
      // printf("pte:v%d\n", *pte & PTE_V);
      struct swappedListNode *t = n->next->next;
      swappedListNodeFree(n->next);
      n->next = t;
      swapDecCount(s);
      if(swapGetCount(s) == 0){
        // release(&swapped.lock);
        // printf("swapfree en\n");
      // // releasesleep(&slock);
        swapfree(s);
      // // acquiresleep(&slock);
        // printf("swapfree ex\n");
        // acquire(&swapped.lock);
      }
      ++rfc;
      addLive(pte, pid, vp, 0);
    }
    else
      n = n->next;
  }

  // release(&swapped.lock);
  acquire(&refCount.lock);
  int ppn = PA2PPN(mem);
  if(refCount.count[ppn] != 1){
    panic("ref count not 0");
  }
  refCount.count[ppn] = rfc;
  release(&refCount.lock);
  // releasesleep(&slock);

  if(debug)
    printf("swapIn ex\n");
}

void removeFromSwapped(int procId, int vpn, pte_t* pte){
  // acquiresleep(&slock);
  if(*pte & PTE_V){
    panic("valid bit is on");
  }
  struct swappedListNode *s , *t;
  // acquire(&swapped.lock);
  s = swapped.list;
  if(s == 0){
    panic("swapped list head empty\n");
  }
  int f = 0;
  while(s->next){
    
    if(s->next->procId == procId && s->next->vpn == vpn && s->next->pte == pte){
      if(s->next->procId == 3 && s->next->vpn == 0){
        panic("removed");
      }
      
      
      f = 1;
      t = s->next;
      s->next = t->next;
      swapDecCount(t->sp);
      if(swapGetCount(t->sp) == 0){
        // release(&swapped.lock);
        // printf("swapfree en\n");
      // // releasesleep(&slock);
        swapfree(t->sp);
      // // acquiresleep(&slock);
        // printf("swapfree ex\n");
        // acquire(&swapped.lock);
      }
      swappedListNodeFree(t);
      break;
    }
    // else if(s->next->procId == 3 && s->next->vpn == 0){
    //   panic("e");
    // }
    else
      s = s->next;
  }
  if(!f){
    printf("looking for id %d vpn %d\n", procId, vpn);
    getLiveCount();
    printf("============\n");
    swapListSize();
    panic("rem from swap: swap not found\n");
  }
  // releasesleep(&slock);
  // release(&swapped.lock);
}

void removeLast(){
  
  // acquire(&live.lock);
  struct liveListNode* t = live.list;
  if(t == 0){
    panic("live list head empty");
  }
  else if(t->next == 0){
    panic("live list empty");
  }
  t = t->next;
  while(t->next){
    t = t->next;
  }
  // printf("t->pid: %d t->vpn: %d t->ppn:%d\n", t->procId, t->vpn, PTE2PPN(*t->pte));
  // release(&live.lock);
  swapOut(t);
}

void addLive(pte_t *pte, int procId, int vpn, int holdSleep){
  // printf("add live en\n");
  // if(holdSleep)
    // acquiresleep(&slock);

  if(procId == 3 && vpn == 0){
    // panic("asd");
    // printf("sdf");
  }
  if(*pte & PTE_SWAPPED){
    panic("swapped bit on");
  }

  // printf("add live en\n");
  struct liveListNode* nd = liveListNodeAlloc();
  if(nd == 0){
    panic("liveListNode alloc");
  }
  nd->procId = procId;
  nd->pte = pte;
  nd->vpn = vpn;
  nd->next = 0;
  // acquire(&live.lock);
  if(live.list == 0){
    panic("list head empty");
  }
  else{
    // struct liveListNode* l = live.list;
    // while(l->next){
    //   if(l->next->procId == procId && l->next->vpn == vpn){
    //     printf("%d %d\n", l->next->procId, l->next->vpn);
    //     panic("entry exists\n");
    //   }
    //   l = l->next;
    // }

    nd->next = live.list->next;
    live.list->next = nd;
  }
  int ppn = PTE2PPN(*pte);
  if(ppn < 0 || ppn >= PAGE_COUNT){
    printf("ppn: %d\n", ppn);
    panic("invalid ppn");
  }
  live.count[ppn] += 1;
  if(live.count[ppn] == 1){
    ++live.liveCount;
  }
  // issue

  if(live.liveCount >= MAX_LIVE_PAGE){
    // release(&live.lock);
    removeLast();
  }
  // else
    // release(&live.lock);
  // printf("add live ex\n");
  // if(holdSleep)
    // releasesleep(&slock);
  // printf("add live ex\n");
  
}

void removeLive(int vpn, int procId, uint64* pte){
  // printf("remove live en\n");
  // acquiresleep(&slock);

  if(*pte & PTE_SWAPPED){
    panic("swapped bit on _");
  }
  if(debug)
    printf("pte v : %d\n", *pte & PTE_V);
  struct liveListNode* n;
  // acquire(&live.lock);
  if(live.list == 0){
    panic("live list head empty\n");
  }
  n = live.list;
  int f = 0;
  while(n->next != 0){
    
    // printf("p: %d vpn: %d\n", n->next->procId, n->next->vpn);
    if(n->next->pte == pte){
      // if(n->next->vpn == 9 && n->next->procId == 4){
      //   panic("removed");
      // }
      struct liveListNode* t = n->next;
      n->next = n->next->next;
      liveListNodeFree(t);
      f = 1;
      
    }
    else
      n = n->next;
  }

  if(!f){
    printf("pte: %p ppn: %d vpn: %d procId: %d swapped:%d\n", pte, PTE2PPN(*pte), vpn, procId, (*pte) & PTE_SWAPPED);
    getLiveCount();
    swapListSize();
    panic("pte not found_");
  }
  
  int ppn = PTE2PPN(*pte);
  live.count[ppn] -= 1;
  if(live.count[ppn] == 0)
    --live.liveCount;
  else if(live.count[ppn] < 0)
    panic("live count < 0");
  // release(&live.lock);
  // printf("remove live ex\n");
  // releasesleep(&slock);

}




int getLiveCount(){
  // // acquiresleep(&slock);
  int c = 0;
  struct liveListNode* n;
  // acquire(&live.lock);
  printf("===========live stats===========\n");
  n = live.list->next;
  while(n != 0){
    if(debug)
      printf("n->procId : %d vpn: %d ppn:%d\n", n->procId, n->vpn, PTE2PPN(*n->pte));
    n = n->next;
    ++c;
  }
  int d = live.liveCount;  
  // release(&live.lock);  
  printf("live list size: %d unique count: %d\n", c, live.liveCount);
  // // releasesleep(&slock);

  return d;
}


void incRefCount(uint64 ppn){
  if(ppn < 0 || ppn >= PAGE_COUNT)
    panic("ref count");
  acquire(&refCount.lock);
  refCount.count[ppn]++;
  release(&refCount.lock);
}
void decRefCount(uint64 ppn){
  if(ppn < 0 || ppn >= PAGE_COUNT)
    panic("ref count");
  acquire(&refCount.lock);
  refCount.count[ppn]--;
  int count = refCount.count[ppn];
  release(&refCount.lock);
  if(count == 0){
    uint64 pa2 = PPN2PA(ppn);
    kfree2((void*)pa2);
  }
}

int getRefCount(uint64 ppn){
  if(ppn < 0 || ppn >= PAGE_COUNT)
    panic("ref count");
  acquire(&refCount.lock);
  int c = refCount.count[ppn];
  release(&refCount.lock);
  return c;
}

void initRefCount(){
  initlock(&refCount.lock, "refCount");
  for(int i = 0; i < PAGE_COUNT; ++i)
      refCount.count[i] = -1;
}

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.


struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initRefCount();
  freerange(end, (void*)PHYSTOP);
  liveListNodeInit();
  swappedListNodeInit();
  liveListInit();
  swappedListInit();
  initsleeplock(&slock, "sleepLock");
}

void
freerange(void *pa_start, void *pa_end)
{
  
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    uint64 ppn = PA2PPN((uint64)p);
    refCount.count[ppn] = 0;
    kfree2((void*)(PPN2PA(ppn)));
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void kfree(void *pa){
  uint64 ppn = PA2PPN((uint64)pa);
  if(getRefCount(ppn) <= 0){
    panic("kfree__");
  }
  decRefCount(ppn);
}

void
kfree2(void *pa)
{
  // printf("freeing stuff\n");
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    uint64 ppn = PA2PPN((uint64)r);
    // printf("allocating page: %d\n", ppn);
    incRefCount(ppn);
    // printf("%d\n", refCount[ppn]);
  }
  return (void*)r;
}

int freePageCountFromFreeList(){
  int c = 0;
  struct run *curr;
  acquire(&kmem.lock);
  curr = kmem.freelist;
  while(curr){
    ++c;
    curr = curr->next;
  }
  release(&kmem.lock);
  return c;
}
int freePageCountFromRefCount(){
  int c = 0;
  for(int i = 0; i < (PAGE_COUNT); ++i){
    acquire(&refCount.lock);
    if(refCount.count[i] == 0) ++c;
    release(&refCount.lock);
  }
  return c;
}

// int getPPN(uint64 addr){
//   return addr - KERNBASE;
// }

void acquireSlock(){
  acquiresleep(&slock);
}
void releaseSlock(){
  releasesleep(&slock);
}