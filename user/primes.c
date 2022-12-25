#include "kernel/types.h"
#include "user/user.h"


int oneStep(int fileDescriptor){
    int r, ret, currPrime;
    ret = read(fileDescriptor, &r, 4);
    if(ret != 0){
        printf("prime: %d\n", r);
        currPrime = r;
        int p[2];
        pipe(p);

        if(fork() == 0){
            close(p[1]);
            close(fileDescriptor);
            oneStep(p[0]);
            close(p[0]);
        }
        else{
            close(p[0]);
            while(read(fileDescriptor, &r, 4) != 0){
                if(r % currPrime != 0){
                    write(p[1], &r, 4);
                }
            }
            close(fileDescriptor);
            close(p[1]);
            wait((int *)0);
        }
    }
    else{
        close(fileDescriptor);
    }

    return 0;
}

int main(){

    int p[2];
    pipe(p);
    if(fork() == 0){
        close(p[1]);
        oneStep(p[0]);
    }
    else{
        close(p[0]);
        for(int i = 2; i < 36; ++i){
            write(p[1], &i, 4);
        }
        close(p[1]);
        wait((int *)0);
    }
    exit(0);

}