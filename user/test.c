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
void test1(){
    int a = 1;
    printf("============= TEST 1: WRITE ON COW PAGE ============\n");
    pagestats(0);
    sbrk(10 * 4096);
    if(fork() == 0){
        // sbrk(-9 * 4096);
        if(fork() == 0){
            sbrk(-10*4096);
            a = 23;
        }
        else{
            wait(0);
            sbrk(10 * 4096);
            if(fork()!=0){
                a = 123;
                wait(0);
            }
            else{
                a = 55;
            }
            sbrk(-10 * 4096);
        }
        a = 2;
        int c = 1;
        printf("From child\na: %d c: %d\n", a, c);
    }
    else{
        sbrk(-9 * 4096);
        wait(0);
        a = 3;
        int c = 2;
        printf("From parent\na: %d c: %d\n", a, c);
        printf("Live page count should increase by 1\n");
        printf("Free page count should decrease by 1\n");
        pagestats(0);
        return;
    }
    exit(0);

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
// throws usertrap cow failed, cow bit not set
void testCowSwapped(){
    printf("======================= COW SWAP TEST ==================\n");
    int a = 4;
    pagestats(0);
    for(int i = 0; i < 50; ++i)
        sbrk(4096);
    printf("_______________ sbrk done _________________\n");
    pagestats(0);
    
    if(fork() == 0){
        sbrk(-50 * 4096);
        pagestats(0);
        a = 5;
        printf("ca:%d\n", a);
        pagestats(0);
    }
    else{
        wait(0);
        printf("pa:%d\n", a);
        pagestats(0);
    }
}
void testSwapped(){
    int a = 20;
    printf("======================= SWAP TEST ==================\n");
    pagestats(0);
    for(int i = 0; i < 56; ++i)
        sbrk(4096);
    pagestats(0);
    a = 25;
    // sbrk(-20 * 4096);
    printf("a: %d\n", a);

}

void testFork(){
    int a = 5;
    for(int i = 0; i < 56; ++i)
        sbrk(4096);
    fork();
    printf("a:%d\n", a);
}

void testFork2(){
    int a = 5;
    // for(int i = 0; i < 50; ++i)
    //     sbrk(4096);
    sbrk(50 * 4096);
    // printf("========================================================================================================sbrk done\n");
    fork();
    // pagestats(0);
    printf("a:%d\n", a);
    printf("========================================================================================================test done\n");

}

void test1dfs(){
    printf("=============== T-E-S-T-0-1 STARTING ===============\n");
    pagestats(0);
    int a = 5;
    int *b =(int*) malloc(SZ * sizeof(int));
    printf("malloc complete\n");
    for(int i = 0; i < SZ; ++i)
        b[i] = i + 1;
    sbrk(38 * 4096);
    printf("sbrk done\n");
    printf("a:%d b[%d]:%d\n", a, 5, b[5]);
    pagestats(0);
    printf("=============== T-E-S-T-0-1 ENDING ===============\n");

}
// gives sched lock panic
void test2(){
    pagestats(0);
    int a = 5;
    int *b =(int*) malloc(SZ * sizeof(int));
    printf("malloc complete\n");
    for(int i = 0; i < SZ; ++i)
        b[i] = i + 1;
    sbrk(39 * 4096);
    printf("sbrk done\n");
    printf("a:%d b[%d]:%d\n", a, 5, b[5]);
    pagestats(0);
    printf("exiting test1\n");
}
void test3(){
    // pagestats(0);
    int b[200];
    for(int i = 0; i < 200; ++i)
        b[i] = i + 1;
    sbrk(45 * 4096);
    if(fork() == 0){
        printf("=============================from child %d==============\n", b[0]);
    }
    else{
        wait(0);
        printf("================from parent===================\n");
    }
}

// sbrk(50 * 4096);
//     pagestats(0);
//     // printf("========================================================================================================sbrk done\n");
//     if(fork()){
//         wait(0);
//     };
//     // // printf("a:%d\n", a);

//     pagestats(0);

//  gives panic swap in : swap not found
    // sbrk(50 * 4096);
    // // pagestats(0);
    // printf("==========================================\n");
    // if(fork()){
    //     wait(0);
    //     fork();
    //     wait(0);
    //     sbrk(80 * 4096);
    //     a = 23;
    // };
    // printf("a:%d\n", a);

// CPU 3 gives swap in not found error
// sbrk(50 * 4096);
//     // pagestats(0);
//     printf("==========================================\n");
//     if(fork()){
//         // wait(0);
//         fork();
//         a = 55;
//         // wait(0);
//         sbrk(80 * 4096);
//         a = 23;
//     };
//     printf("a:%d\n", a);

    // sbrk(50 * 4096);
    // // pagestats(0);
    // printf("==========================================\n");
    // if(fork()){
    //     // wait(0);
    //     fork();
    //     a = 55;
    //     // wait(0);
    //     sbrk(80 * 4096);
    //     a = 23;
    // };
    // printf("a:%d\n", a);

void swapTestBasic(){
    int n = 30;
    char *pa = sbrk(n * PGSIZE);
    for(int i = 0; i < n ; i++){
        pa[i * PGSIZE] = i;
    }
    for(int i = 0; i < n ; i++){
        if(pa[i * PGSIZE] != i){
            printf("issue\n");
        }
    }
    
    // pagestats(1);
}
void swapTestFork(){
    int n = 60;
    int pgsize = 4096;
    char* pa = sbrk(n * pgsize);
    for(int i = 0; i < n ; i++){
        pa[i * PGSIZE] = i;
    }

    for(int i = 0; i < n; i++){
        printf("%d\n", pa[i * PGSIZE]);
    }
    wait(0);
    pagestats(0);
}

int main(int argc, char** argv){
    // if(argc < 2){
    //     printf("command line arguments needed\n");
    //     exit(1);
    // }
    // if(strcmp("cow", argv[1]) == 0)
    //     cowtest();
    // else if(strcmp("paging", argv[1]) == 0){
    //     pagestats(1);
    // }
    swapTestBasic();
    // swapTest1();
    // sbrk(60 * 4096);
    // test1();
    // pingpong();
    // testSwapped();
    // testCowSwapped();
    // testFork2();
    // int a = 1;

    // sbrk(50 * 4096);
    // // pagestats(0);
    // printf("==========================================\n");
    // if(fork()){
    //     sbrk(-50 * 4096);
    //     // wait(0);
    //     fork();
    //     a = 55;
    //     // wait(0);
    //     sbrk(80 * 4096);
    //     a = 23;
    // };
    // printf("a:%d\n", a);
    // sbrk(60 * 4096);
    // // for(int i = 0; i < 100; ++i)
    // //     sbrk(4096);
    // // sbrk(100 * 4096);
    // // pagestats(0);
    // // printf("==========================================\n");
    // // if(fork()){
    // //     // wait(0);
    // //     fork();
    // //     a = 55;
    // //     // wait(0);
    // //     sbrk(80 * 4096);
    // //     a = 23;
    // // };
    // // if(fork() != 0){
    // //     printf("fork parent\n");
    // //     // wait(0);
    // // }
    // if(fork()){
    //     wait(0);
    //     a = 2;
    //     printf("a: %d\n", a);
    //     if(fork()){
    //         wait(0);
    //         int b = 4;
    //         sbrk(55 * 4096);
    //         b = 23;
    //         a = 3;
    //         printf("a: %d b: %d\n", a, b);
    //     }
        
    // }
    
    // pagestats(0);
    // printf("a:%d\n", a);

    // pagestats(0);
    // printf("========================================================================================================test done\n");
    
    // printf("hello world");
    // pagestats(0);
    // int a = 5;
    // int *b =(int*) malloc(SZ * sizeof(int));
    // for(int i = 0; i < SZ; ++i)
    //     b[i] = i + 1;
    // // int a[20000];
    // // for(int i = 0; i < 20000; ++i){
    // //     a[i] = 0;
    // // }
    // // int b = 3;
    // // pagestats(0);
    // // printf("a[%d]: %d\n", 0, a[0]);
    // // printf("===============================\n");
    // // sbrk(55 * 4096 );
    // // pagestats(0);
    // // sbrk(3 * 4096);
    // // pagestats(0);
    // printf("a:%d b[%d]:%d\n", a, 5, b[5]);

    // if(fork() == 0){
    //     if(fork() != 0){

    //         wait(0);
    //         pagestats(0);

    //     }
    //     else{
    //         // a = 8;
    //         // printf("a:%d\n", a);
    //         pagestats(0);
    //     }
    //     // a = 3;
    //     // printf("a:%d\n", a);
    //     // pagestats(0);
    // }
    // else{
    //     wait(0);
    //     pagestats(0);
    //     printf("===============================\n");
    // }

    // pagestats(0);
    // if(fork() != 0){
    //     wait(0);
    //     pagestats(0);
    // }
    // else{
    //     int a = 5;
    //     printf("a:%d\n", a);
    //     pagestats(0);
    // }

    // fork();
    // int a = 5;
    // printf("%d", a);
    // if(fork()){
    //     wait(0);
    //     a = 1;
    //     b = 2;
    //     // int c = 23;
    //     // pagestats(0);
    //     // printf("p stats from p : %d\n",pagestats(0));
    //     // printf("from parent.c = %d\n",c);
    // }
    // else{
    //     int* c = (int *) malloc(sizeof(int));
    //     *c = 25;        
    //     a = 3;
    //     b = 4;
    //     // printf("from child.c = %d\n",*c);
    //     free(c);

    // }
    // printf("a:%d b:%d\n", a, b);
    // pagestats(0);
    return 0;
}