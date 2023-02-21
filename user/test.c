#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SZ 2000

void test1(){
    pagestats();
    int a = 5;
    int *b =(int*) malloc(SZ * sizeof(int));
    printf("malloc complete\n");
    for(int i = 0; i < SZ; ++i)
        b[i] = i + 1;
    // sbrk(35 * 4096);
    printf("sbrk done\n");
    printf("a:%d b[%d]:%d\n", a, 5, b[5]);
    pagestats();
    printf("exiting test1\n");
}

int main(){
    test1();
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