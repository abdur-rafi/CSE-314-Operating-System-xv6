#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){

    // printf("hello world");
    int a = 5;
    int b = 3;

    if(fork()){
        wait(0);
        a = 1;
        b = 2;
        int c = 23;
        printf("from parent.c = %d\n",c);
    }
    else{
        int* c = (int *) malloc(sizeof(int));
        *c = 25;        
        a = 3;
        b = 4;
        printf("from child.c = %d\n",*c);

    }
    printf("%d %d\n", a, b);
    return 0;
}