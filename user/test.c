#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){

    // printf("hello world");
    int a = 5;
    int b = 3;

    if(fork()){
        a = 1;
        b = 2;
    }
    else{
        a = 3;
        b = 4;
    }
    printf("%d %d\n", a, b);
    printf("done\n");
    return 0;
}