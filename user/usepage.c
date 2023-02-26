#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"


int main(int argc, char** argv){
    if(argc < 2){
        printf("please provide number of pages to use\n");
        exit(1);
    }
    int c = atoi(argv[1]);
    printf("c: %d\n", c);
    char* pa = sbrk(c * PGSIZE);

    for(int i = 0; i < 100000; ++i){
        for(int j = 0; j < 50000; ++j)
            pa[(i % c) * PGSIZE] = 'a';
    }


    return 0;
}