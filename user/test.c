#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){

    // printf("hello world");
    int a = 5;
    // int b = 3;
    // pagestats();
    sbrk(10 * 4096 );
    pagestats();
    if(fork() == 0){
        a = 3;
        printf("a:%d\n", a);
        pagestats();
    }
    else
        wait(0);

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