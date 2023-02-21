// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define MAX_LIVE_PAGE 50
#define QUEUE_SIZE (MAX_LIVE_PAGE) * (NPROC)

#define IN_QUEUE 1
#define SWAPPED 2
#define FREE 3
void kfree2(void *pa);
void removeLast();

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
  int refCount;
  struct swap* sp;
  struct swappedListNode* next;
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
  s->refCount = 0;
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
  struct spinlock lock;
  int count[PAGE_COUNT];
  int liveCount;
} live;

struct {
  struct swappedListNode* list;
  struct spinlock lock;
} swapped;

void liveListInit(){
  initlock(&live.lock, "livelock");
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
  initlock(&swapped.lock, "swappedlock");
  swapped.list = swappedListNodeAlloc();
  if(swapped.list == 0){
    panic("swapped list init");
  }
  else{
    swapped.list->next = 0;
    swapped.list->procId = 0;
    swapped.list->refCount = 0;
    swapped.list->sp = 0;
    swapped.list->vpn = 0;
  }
}
// with liveList lock held
//  issue
void swapOut(struct liveListNode* n){
  // if(!holding(&live.lock)){
  //   panic("liveList lock not held");
  // }
  printf("swapOut en\n");
  if(n == 0){
    panic("null pointer to swap");
  }
  struct swappedListNode* sn = swappedListNodeAlloc();
  if(sn == 0){
    panic("swapListNode not allocated");
  }
  struct swap* s = swapalloc();
  if(s == 0){
    panic("swap not allocated");
  }

  swapout(s,(char*) PTE2PA(*n->pte));
  int rc = 0;
  
  sn->procId = n->procId;
  sn->sp = s;
  sn->next = 0;

  int f = 0;
  acquire(&live.lock);
  int removedPpn = PTE2PPN(*n->pte);
  struct liveListNode* l = live.list;
  if(l == 0){
    panic("live list head empty\n");
  }
    // while(l && )
  while(l->next){
    int lppn = PTE2PPN(*l->next->pte);
    if(removedPpn == lppn){
      f = 1;
      *l->next->pte &= (~PTE_V);
      *l->next->pte |= PTE_SWAPPED;
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
    panic("pte not found\n");
  }
  
  // if(PTE2PPN(l->pte) == removedPpn){
  //   *l->pte &= (~PTE_V);
  //   *l->pte |= PTE_SWAPPED;
  //   liveListNodeFree(l);
  //   live.list = 0;
  // }
  live.liveCount--;
  
  release(&live.lock);
  sn->refCount = rc;

  
  acquire(&refCount.lock);
  refCount.count[removedPpn] = 0;
  release(&refCount.lock);
  kfree2((void*) PPN2PA(removedPpn));
  // issue
  acquire(&swapped.lock);
  if(swapped.list == 0){
    panic("list head empty");
  }
  else{
    
    sn->next = swapped.list->next;
    swapped.list->next = sn;
  }
  release(&swapped.lock);

  printf("swapOut ex\n");

}

void swapIn(int vpn, int procId, uint64 *pte){

  // release space
  printf("swapIn en\n");
  acquire(&live.lock);
  while(live.liveCount >= MAX_LIVE_PAGE){
    release(&live.lock);
    removeLast();
    acquire(&live.lock);
  }
  release(&live.lock);

  struct swappedListNode* n;
  acquire(&swapped.lock);
  n = swapped.list;
  if(n == 0){
    panic("swap list empty\n");
  }
  int f = 0;
  while(n->next){
    if(n->next->procId == procId && n->next->vpn == vpn){
      f = 1;
      if(n->next->refCount == 0){
        panic("ref count is 0");
      }
      struct swappedListNode *t;
      release(&swapped.lock);

      char* mem =(char *) kalloc();
      swapin(mem,n->next->sp);
      *pte = (PTE_FLAGS(*pte)) | (PA2PTE((uint64)mem)) | (PTE_V);
      if(*pte | PTE_COW){
        *pte &= (~PTE_COW);
        *pte |= PTE_W;
      }
      *pte &= (~PTE_SWAPPED);
      addLive(pte, procId, vpn);
      
      acquire(&swapped.lock);
      n->next->refCount -= 1;
      if(n->next->refCount == 0){
        t = n->next->next;
        release(&swapped.lock);

        swapfree(n->next->sp);
        swappedListNodeFree(n->next);
        
        acquire(&swapped.lock);
        n->next = t;
      }
      break;
    }
    else{
      printf("procId : %d\n", n->next->procId);
      n = n->next;
    }
  }
  if(!f){
    panic("swap not found\n");
  }
  release(&swapped.lock);

  printf("swapIn ex\n");
}

void removeLast(){
  acquire(&live.lock);
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
  release(&live.lock);
  swapOut(t);
}

void addLive(pte_t *pte, int procId, int vpn){
  // printf("add live en\n");
  struct liveListNode* nd = liveListNodeAlloc();
  if(nd == 0){
    panic("liveListNode alloc");
  }
  nd->procId = procId;
  nd->pte = pte;
  nd->vpn = vpn;
  nd->next = 0;
  acquire(&live.lock);
  if(live.list == 0){
    panic("list head empty");
  }
  else{
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
    release(&live.lock);
    removeLast();
  }
  else
    release(&live.lock);
  // printf("add live ex\n");

}

void removeLive(uint64* pte){
  // printf("remove live en\n");
  struct liveListNode* n;
  acquire(&live.lock);
  if(live.list == 0){
    panic("live list head empty\n");
  }
  n = live.list;
  int f = 0;
  while(n->next != 0){
    if(n->next->pte == pte){
      struct liveListNode* t = n->next;
      n->next = n->next->next;
      liveListNodeFree(t);
      f = 1;
    }
    else
      n = n->next;
  }

  if(!f){
    panic("pte not found");
  }
  
  int ppn = PTE2PPN(*pte);
  live.count[ppn] -= 1;
  if(live.count[ppn] == 0)
    --live.liveCount;
  else if(live.count[ppn] < 0)
    panic("live count < 0");
  release(&live.lock);
  // printf("remove live ex\n");
}




int getLiveCount(){
  int c = 0;
  struct liveListNode* n;
  acquire(&live.lock);
  n = live.list->next;
  while(n != 0){
    // printf("n->procId : %d\n", n->procId);
    n = n->next;
    ++c;
  }
  int d = live.liveCount;  
  release(&live.lock);  
  printf("live list size: %d unique count: %d\n", c, live.liveCount);
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

int pagestats(){
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
int pagestatsFromRefCount(){
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