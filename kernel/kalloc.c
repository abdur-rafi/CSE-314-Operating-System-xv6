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


struct run {
  struct run *next;
};


struct node {
  pte_t *pte;
  int procId;
  int vpn;
  struct node* next;
};


struct {
  struct spinlock lock;
  struct run *freelist;
} nodemem;

void
nodeinit(void)
{
  initlock(&nodemem.lock, "nodemem");
  nodemem.freelist = 0;
}

struct node *
nodealloc(void)
{
  struct run *r;
  struct node *s;

  acquire(&nodemem.lock);
  r = nodemem.freelist;
  if(!r){
    release(&nodemem.lock);
    char *mem = kalloc();
    char *mem_end = mem + PGSIZE;
    for(; mem + sizeof(struct node) <= mem_end; mem += sizeof(struct node)){
      r = (struct run*)mem;

      acquire(&nodemem.lock);
      r->next = nodemem.freelist;
      nodemem.freelist = r;
      release(&nodemem.lock);
    }
    acquire(&nodemem.lock);
    r = nodemem.freelist;
  }
  nodemem.freelist = r->next;
  release(&nodemem.lock);
  
  s = (struct node*)r;

  
  return s;
}
void
nodefree(struct node *s)
{
  struct run *r;

  if(!s)
    panic("swapfree");
  r = (struct run*)s;
  acquire(&nodemem.lock);
  r->next = nodemem.freelist;
  nodemem.freelist = r;
  release(&nodemem.lock);
}
struct {
  struct node* list;
  struct spinlock lock;
  int count[PAGE_COUNT];
  int liveCount;
} live;

void liveinit(){
  initlock(&live.lock, "livelock");
  live.list = 0;
  for(int i = 0; i < PAGE_COUNT; ++i){
    live.count[i] = 0;
  }
  live.liveCount = 0;
}

void addLive(pte_t *pte, int procId, int vpn){
  struct node* nd = nodealloc();
  if(nd == 0){
    panic("node alloc");
  }
  nd->procId = procId;
  nd->pte = pte;
  nd->vpn = vpn;
  nd->next = 0;
  acquire(&live.lock);
  if(live.list == 0){
    live.list = nd;
  }
  else{
    nd->next = live.list;
    live.list = nd;
  }
  int ppn = PTE2PPN(*pte);
  live.count[ppn] += 1;
  if(live.count[ppn] == 1){
    ++live.liveCount;
  }
  release(&live.lock);
}

void removeLive(uint64* pte){
  struct node* n;
  acquire(&live.lock);
  n = live.list;
  if(n == 0){
    panic("live list empty");
  }
  else if(n->pte == pte){
    live.list = n->next;
    nodefree(n);
  }
  else{
    int f = 0;
    while(n->next != 0){
      if(n->next->pte == pte){
        struct node* t = n->next;
        n->next = n->next->next;
        nodefree(t);
        f = 1;
        
        if(n->next == 0) break;
      }
      n = n->next;
    }

    if(!f){
      panic("pte not found");
    }
    
  }
  int ppn = PTE2PPN(*pte);
  live.count[ppn] -= 1;
  if(live.count[ppn] == 0)
    --live.liveCount;
  else if(live.count[ppn] < 0)
    panic("live count < 0");
  release(&live.lock);
}

void kfree2(void *pa);

struct {
  char count[PAGE_COUNT];
  struct spinlock lock;
} refCount;


int getLiveCount(){
  int c = 0;
  struct node* n;
  acquire(&live.lock);
   n = live.list;
  while(n != 0){
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
  nodeinit();
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