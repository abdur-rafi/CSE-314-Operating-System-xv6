#include "kernel/types.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
    if(argc < 2){
        printf("Error: No argument given");
    }
    else{
        int n = atoi(argv[1]);
        if(sleep(n) == 0){
            exit(0);
        }
        else
            exit(1);
    }
    exit(1);

}
