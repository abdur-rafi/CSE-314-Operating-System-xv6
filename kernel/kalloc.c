// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define PAGE_COUNT 32 * 1024
#define MAX_LIVE_PAGE 50
#define QUEUE_SIZE MAX_LIVE_PAGE * NPROC

#define IN_QUEUE 1
#define SWAPPED 2
#define FREE 3

char refCount[PAGE_COUNT];
char pageStatus[PAGE_COUNT];
struct swap* swapped[PAGE_COUNT];

int getPageStatus(int ppn){
  return pageStatus[ppn];
}
void setPageStatus(int ppn, int status){
  pageStatus[ppn] = status;
}
void swapPage(){
  // pte_t* pte = q.arr[q.f];
}

struct Queue{
  pte_t* arr[QUEUE_SIZE], *temp[QUEUE_SIZE];
  int f;
  int uniqueCount;
  int entryCount;
} q;

void removeFromQueue(int, int);

void evictPage(int ppn){
  if(getPageStatus(ppn) != IN_QUEUE){
    panic("evict page");
  }
  swapped[ppn] = swapalloc();
  if(swapped[ppn] == 0){
    panic("swapp");
  }
  // printf("%d\n", sizeof(char *));
  swapout(swapped[ppn],(char *) PPN2PA(ppn));
  // printf("evicted\n");
  removeFromQueue(ppn, 1);
  setPageStatus(ppn, SWAPPED);
  // kfree2(PPN2PA(ppn));
  printf("evicted\n");
}

void removeFromQueue(int ppn, int markInvalid){
  if(getPageStatus(ppn) != IN_QUEUE)
    return ;
  int j = 0;
  for(int i = 0; i < q.entryCount; ++i){
    int index = (q.f + i) % QUEUE_SIZE;
    pte_t *pte = q.arr[index];
    if(PTE2PPN(*pte) != ppn){
      q.temp[j++] = pte;
    }
    else if(markInvalid){
      *pte = ((*pte) & (~PTE_V)) | (PTE_SWAPPED);
    }
  }
  q.f = 0;
  q.entryCount = j;
  for(int i = 0; i < j; ++i)
    q.arr[i] = q.temp[i];
  --q.uniqueCount;
}

int dequeue(){
  if(q.uniqueCount <= 0){
    panic("dequeue");
  }
  int i = q.f;
  q.f = (q.f + 1) % QUEUE_SIZE;
  return PTE2PPN(*q.arr[i]);
}

void freePage(int ppn){
  if(getPageStatus(ppn) == IN_QUEUE){
    removeFromQueue(ppn, 0);
    setPageStatus(ppn, FREE);
  }
}

void enqueue(pte_t *pte){
  int ppn = PTE2PPN(*pte);
  if(q.uniqueCount == MAX_LIVE_PAGE){
    evictPage(dequeue());
  }
  int index = (q.f + q.entryCount) % QUEUE_SIZE;
  q.entryCount++;
  q.arr[index] = pte;
  int status = getPageStatus(ppn);
  if(status == FREE){
    setPageStatus(ppn, IN_QUEUE);
    q.uniqueCount++;
  }
  else if(status == IN_QUEUE){
  }
  else if(status == SWAPPED){
    // *pte = (*pte) & (~PTE_V);
    // q.entryCount--;
  }
  else{
    // printf("%d issue\n", ppn);
  }
  // printf("%d %d\n", q.entryCount, q.liveCount);
}

int getLiveCount(){
  // printf("entry count : %d\n", q.entryCount);
  return q.uniqueCount;
}

void kfree2(void *pa);

void incRefCount(uint64 ppn){

  refCount[ppn]++;
}
void decRefCount(uint64 ppn){
  refCount[ppn]--;
  if(refCount[ppn] == 0){
    uint64 pa2 = PPN2PA(ppn);
    kfree2((void*)pa2);
    freePage(ppn);
    // if(getPageStatus(ppn) == IN_QUEUE){
    //   --q.uniqueCount;
    // }
    // setPageStatus(ppn, FREE);
  }
  else if(refCount[ppn] < 0){
    printf("error\n");
  }
}

int getRefCount(uint64 ppn){
  return refCount[ppn];
}

void initRefCount(){
  for(int i = 0; i < PAGE_COUNT; ++i)
      refCount[i] = -1;
}

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

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
}

void
freerange(void *pa_start, void *pa_end)
{
  
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    uint64 ppn = PA2PPN((uint64)p);
    refCount[ppn] = 0;
    pageStatus[ppn] = FREE;
    kfree2((void*)(PPN2PA(ppn)));
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void kfree(void *pa){
  uint64 ppn = PA2PPN((uint64)pa);
  if(refCount[ppn] <= 0){
    panic("kfree2");
  }
  decRefCount(ppn);
  // kfree2(pa);

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
    if(refCount[i] == 0) ++c;
  }
  return c;
}

// int getPPN(uint64 addr){
//   return addr - KERNBASE;
// }