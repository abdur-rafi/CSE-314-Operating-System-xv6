#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){

    // printf("hello world");
    int a = 5;
    fork();
    int b = 6;
    a = 8;
    printf("%d %d\n", a, b);
    return 0;
}