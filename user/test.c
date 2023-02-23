#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SZ 200
// gives sched lock panic
// now gives cow failed
// now gives panic acquire
void test1(){
    int a = 1;
    printf("============= TEST 1: WRITE ON COW PAGE ============\n");
    pagestats();
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
        pagestats();
        return;
    }
    exit(0);

}

void pingpong()
{
    printf("============== PING PONG TEST ================\n");
    int p1[2], p2[2];
    // p1 -> parent to child
    // p2 -> child to parent
    pipe(p1);
    pipe(p2);
    pagestats();
    char message[30] = {0};
    if(fork() == 0){
        read(p1[0], message, 29);
        printf("%d: received ping %s\n",getpid(), message);
        write(p2[1], "hello from child",15);
        exit(0);
    }
    else{
        write(p1[1], "hello from parent",17);
        read(p2[0], message, 29);
        printf("%d: received pong %s\n",getpid(),message);
        wait(0);
    }
    pagestats();

}
// throws usertrap cow failed, cow bit not set
void testCowSwapped(){
    printf("======================= COW SWAP TEST ==================\n");
    int a = 4;
    pagestats();
    for(int i = 0; i < 50; ++i)
        sbrk(4096);
    printf("_______________ sbrk done _________________\n");
    pagestats();
    
    if(fork() == 0){
        sbrk(-50 * 4096);
        pagestats();
        a = 5;
        printf("ca:%d\n", a);
        pagestats();
    }
    else{
        wait(0);
        printf("pa:%d\n", a);
        pagestats();
    }
}

void test1dfs(){
    printf("=============== T-E-S-T-0-1 STARTING ===============\n");
    pagestats();
    int a = 5;
    int *b =(int*) malloc(SZ * sizeof(int));
    printf("malloc complete\n");
    for(int i = 0; i < SZ; ++i)
        b[i] = i + 1;
    sbrk(38 * 4096);
    printf("sbrk done\n");
    printf("a:%d b[%d]:%d\n", a, 5, b[5]);
    pagestats();
    printf("=============== T-E-S-T-0-1 ENDING ===============\n");

}
// gives sched lock panic
void test2(){
    pagestats();
    int a = 5;
    int *b =(int*) malloc(SZ * sizeof(int));
    printf("malloc complete\n");
    for(int i = 0; i < SZ; ++i)
        b[i] = i + 1;
    sbrk(39 * 4096);
    printf("sbrk done\n");
    printf("a:%d b[%d]:%d\n", a, 5, b[5]);
    pagestats();
    printf("exiting test1\n");
}
void test3(){
    // pagestats();
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
int main(){
    // test1();
    // pingpong();
    // testCowSwapped();
    // printf("hello world");
    // pagestats();
    // int a = 5;
    // int *b =(int*) malloc(SZ * sizeof(int));
    // for(int i = 0; i < SZ; ++i)
    //     b[i] = i + 1;
    // // int a[20000];
    // // for(int i = 0; i < 20000; ++i){
    // //     a[i] = 0;
    // // }
    // // int b = 3;
    // // pagestats();
    // // printf("a[%d]: %d\n", 0, a[0]);
    // // printf("===============================\n");
    // // sbrk(55 * 4096 );
    // // pagestats();
    // // sbrk(3 * 4096);
    // // pagestats();
    // printf("a:%d b[%d]:%d\n", a, 5, b[5]);

    // if(fork() == 0){
    //     if(fork() != 0){

    //         wait(0);
    //         pagestats();

    //     }
    //     else{
    //         // a = 8;
    //         // printf("a:%d\n", a);
    //         pagestats();
    //     }
    //     // a = 3;
    //     // printf("a:%d\n", a);
    //     // pagestats();
    // }
    // else{
    //     wait(0);
    //     pagestats();
    //     printf("===============================\n");
    // }

    // pagestats();
    // if(fork() != 0){
    //     wait(0);
    //     pagestats();
    // }
    // else{
    //     int a = 5;
    //     printf("a:%d\n", a);
    //     pagestats();
    // }

    // fork();
    // int a = 5;
    // printf("%d", a);
    // if(fork()){
    //     wait(0);
    //     a = 1;
    //     b = 2;
    //     // int c = 23;
    //     // pagestats();
    //     // printf("p stats from p : %d\n",pagestats());
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
    // pagestats();
    return 0;
}