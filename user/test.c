#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"
#define SZ 200
// gives sched lock panic
// now gives cow failed
// now gives panic acquire

void pingpong();
void cowtest(){
    
    int c = 4;
    int pc; 
    int cc1, cc2;
    char* pa = sbrk(c * PGSIZE);
    for(int i = 0; i < c; i++){
        pa[i * PGSIZE] = ('a' + i);
    }
    
    
    pc = pagestats(0);

        if(fork() == 0){
        cc1 = pagestats(0);
        for(int i = 0; i < c; i++){
            printf("a[%d] : %d\n", i * PGSIZE, pa[i * PGSIZE]);
        }
        printf("free pages should decrease by 1\n");
        cc2 = pagestats(0);
        if(cc2 != cc1 - 1){
            printf("----------failed-----------\n");
            exit(1);
        }
        else{
            printf("-----------ok------------\n");
        }
        sbrk(-c * PGSIZE);
        exit(0);
    }
    else{
        wait(0);
        printf("the pages should be accessible after child's sbrk\n");
        for(int i = 0; i < c; i++){
            printf("a[%d] : %d\n", i * PGSIZE, pa[i * PGSIZE]);
        }
        printf("page count should match the first call\n");
        cc1 = pagestats(0);
        if(cc1 != pc){
            printf("----------failed-----------\n");
            exit(1);
        }
        else{
            printf("-----------ok------------\n");
        }
    }


    if(fork() == 0){
        cc1 = pagestats(0);
        
        printf("writing to pages should decrease free page count by %d + 1 (from loop var and printf) \n", c);
        for(int i = 0; i < c; i++){
            pa[i * PGSIZE] = ('c' + i);
        }
        cc2 = pagestats(0);
        if(cc1 != cc2 + c + 1){
            printf("----------failed-----------\n");
            exit(1);
        }
        else{
            printf("-----------ok------------\n");
        }
        exit(0);
    }
    else{
        
        wait(0);
        cc1 = pagestats(0);
        for(int i = 0; i < c; i++){
            pa[i * PGSIZE] = ('b' + i);
        }
        printf("writing to it should not decrease page count now\n");
        cc2 = pagestats(0);
        if(cc2 != cc1){
            printf("----------failed-----------\n");
            exit(1);
        }
        else{
            printf("-----------ok------------\n");
        }
    }

    cc1 = pagestats(0);
    if(cc1 != pc){
        printf("----------failed-----------\n");
        exit(1);
    }
    else{
        printf("-----------ok------------\n");
    }
    printf("doing this test multiple times should show the same count, indicating proper clean un of the process\n");

    pingpong();

}

void pingpong()
{
    printf("============== PING PONG TEST ================\n");
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);
    char message[30] = {0};
    pagestats(0);
    write(p1[1], "hello from parent",17);
    if(fork() == 0){
        read(p1[0], message, 29);
        printf("%d: received ping %s\n",getpid(), message);
        write(p2[1], "hello from child",15);
        exit(0);
    }
    else{
        read(p2[0], message, 29);
        printf("%d: received pong %s\n",getpid(),message);
        wait(0);
    }
    pagestats(0);

}

void swapTestBasic(){
    int n = 20;
    char *pa = sbrk(n * PGSIZE);
    for(int i = 0; i < n ; i++){
        pa[i * PGSIZE] = i;
    }
    for(int i = 0; i < n ; i++){
        if(pa[i * PGSIZE] != i){
            printf("issue %d\n", i);
        }
    }
    pagestats(1);
}
void swapTestFork(){
    int n = 30;
    int pgsize = 4096;
    char* pa = sbrk(n * pgsize);
    pagestats(1);
    for(int i = 0; i < n ; i++){
        pa[i * PGSIZE] = i;
    }
    int r = fork();
    for(int i = 0; i < n; i++){
        if(pa[i * PGSIZE] != i){
            printf("issue with block: %d\n", i);
        }
    }
    wait(0);
    
    if(r) exit(0);

    if(fork() == 0){
        for(int i = 0; i < n; i++){
            pa[i * PGSIZE] = i + 1;
        }
        for(int i = 0; i < n; i++){
            if(pa[i * PGSIZE] != i + 1){
                printf("issue with block: %d\n", i);
            }
        }
        exit(0);
    }
    else{
        wait(0);
    }
    if(r != 0){
        wait(0);
        for(int i = 0; i < n; i++){
            if(pa[i * PGSIZE] != i){
                printf("issue with block: %d\n", i);
            }
        }
        pagestats(1);
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("command line arguments needed\n");
        exit(1);
    }
    if(strcmp("cow", argv[1]) == 0)
        cowtest();
    else if(strcmp("paging", argv[1]) == 0){
        pagestats(1);
        printf("--------------- basic test --------------\n");
        swapTestBasic();
        printf("--------------- Fork cow test --------------\n");
        swapTestFork();
    }
    
    return 0;
}