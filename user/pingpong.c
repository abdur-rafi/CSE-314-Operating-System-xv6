#include "kernel/types.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
    int p1[2], p2[2];
    // p1 -> parent to child
    // p2 -> child to parent
    pipe(p1);
    pipe(p2);

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
        exit(0);
    }

}
