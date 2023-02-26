struct sleeplock;
struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct stat;
struct superblock;
struct swap;

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);
void            bpin(struct buf*);
void            bunpin(struct buf*);

// console.c
void            consoleinit(void);
void            consoleintr(int);
void            consputc(int);

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, uint64, int n);
int             filestat(struct file*, uint64 addr);
int             filewrite(struct file*, uint64, int n);

// fs.c
void            fsinit(int);
uint            balloc(uint dev);
void            bfree(int dev, uint b);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit();
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, int, uint64, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, int, uint64, uint, uint);
void            itrunc(struct inode*);

// ramdisk.c
void            ramdiskinit(void);
void            ramdiskintr(void);
void            ramdiskrw(struct buf*);

// kalloc.c
void*           kalloc(void);
void            kfree(void *);
void            kinit(void);
void            incRefCount(uint64 ppn);
void            decRefCount(uint64 ppn);
int             getRefCount(uint64 ppn);
int             freePageCountFromFreeList();
int             freePageCountFromRefCount();
void            addLive(pte_t *pte, int procId, int vpn, int);
int             getLiveCount();
void            removePTE(pte_t *);
void            removeLive(int vpn, int procId, pte_t* );
void            swapIn(int vpn, int procId, uint64 *pte);
void            swapListSize();
void            releaseSlock();
void            acquireSlock();
// log.c
void            initlog(int, struct superblock*);
void            log_write(struct buf*);
void            begin_op(void);
void            end_op(void);


// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, uint64, int);
int             pipewrite(struct pipe*, uint64, int);

// printf.c
void            printf(char*, ...);
void            panic(char*) __attribute__((noreturn));
void            printfinit(void);

// proc.c
int             cpuid(void);
void            exit(int);
int             fork(void);
int             growproc(int);
void            proc_mapstacks(pagetable_t);
pagetable_t     proc_pagetable(struct proc *);
void            proc_freepagetable(pagetable_t, uint64, int);
int             kill(int);
int             killed(struct proc*);
void            setkilled(struct proc*);
struct cpu*     mycpu(void);
struct cpu*     getmycpu(void);
struct proc*    myproc();
void            procinit(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(uint64);
void            wakeup(void*);
void            yield(void);
int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void            procdump(void);
void            pageCountOfProcs();
void            releaseLock();
void            acquireLock();
// swap.c
void            swapinit(void);
void            swapfree(struct swap*);
struct swap*    swapalloc(void);
void            swapout(struct swap *dst_sp, char *src_pa);
void            swapin(char *dst_pa, struct swap *src_sp);
void            swapSetRefCount(struct swap* s, int c);
void            swapIncCount(struct swap* s);
void            swapDecCount(struct swap* s);
int             swapGetCount(struct swap* s);
void            removeFromSwapped(int procId, int vpn, pte_t*);



// swtch.S
void            swtch(struct context*, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            push_off(void);
void            pop_off(void);

// sleeplock.c
void            acquiresleep(struct sleeplock*);
void            releasesleep(struct sleeplock*);
int             holdingsleep(struct sleeplock*);
void            initsleeplock(struct sleeplock*, char*);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// syscall.c
void            argint(int, int*);
int             argstr(int, char*, int);
void            argaddr(int, uint64 *);
int             fetchstr(uint64, char*, int);
int             fetchaddr(uint64, uint64*);
void            syscall();

// trap.c
extern uint     ticks;
void            trapinit(void);
void            trapinithart(void);
extern struct spinlock tickslock;
void            usertrapret(void);

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);
void            uartputc_sync(int);
int             uartgetc(void);

// vm.c
void            kvminit(void);
void            kvminithart(void);
void            kvmmap(pagetable_t, uint64, uint64, uint64, int);
int             mappages(pagetable_t, uint64, uint64, uint64, int,int,int);
pagetable_t     uvmcreate(void);
void            uvmfirst(pagetable_t, uchar *, uint, int);
uint64          uvmalloc(pagetable_t, uint64, uint64, int, int);
uint64          uvmdealloc(pagetable_t, uint64, uint64, int);
int             uvmcopy(pagetable_t, pagetable_t, uint64, int, int,struct spinlock*);
void            uvmfree(pagetable_t, uint64, int);
void            uvmunmap(pagetable_t, uint64, uint64, int, int);
void            uvmclear(pagetable_t, uint64);
pte_t *         walk(pagetable_t, uint64, int,int);
uint64          walkaddr(pagetable_t, uint64, int);
int             copyout(pagetable_t, uint64, char *, uint64, int);
int             copyin(pagetable_t, char *, uint64, uint64, int);
int             copyinstr(pagetable_t, char *, uint64, uint64, int);
int             assignPagesOnWrite(pagetable_t p, int);
void            pageCount(pagetable_t, int, int*, int*, int *, int *);
int             getSwappedPage(pagetable_t p, int procId);
void            addSwapped(pte_t *pte, int oldProcId, int newProcId, int vpn);
void            swapListSize();

// plic.c
void            plicinit(void);
void            plicinithart(void);
int             plic_claim(void);
void            plic_complete(int);

// virtio_disk.c
void            virtio_disk_init(void);
void            virtio_disk_rw(struct buf *, int);
void            virtio_disk_intr(void);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
