#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"


int setTickets(int n){
    if(settickets(n) == -1){
        printf("set tickets of process: %d failed\n", getpid());
        return -1;
    }
    else{
        printf("tickets of process: %d set to %d\n", getpid(), n);
    }
    return 1;
}

int main(int argc, char** argv){
    int tickets = 1;
    if(argc < 2){
        printf("default ticket number %d used\n",tickets );
    }
    else{
        tickets = atoi(argv[1]);
    }
    setTickets(tickets);


    // while(1);


    // if(fork() == 0){
    //     // settickets(5);
    //     setTickets(tickets * 3);
        
    //     if(fork() == 0){
    //         setTickets(tickets > 1 ? (tickets / 2) : 1);
    //         // settickets(3);
    //     }

    //     if(fork() == 0){
    //         setTickets(tickets * 2);
    //         // settickets(3);
    //     }
    //     // if(fork() == 0){
    //     //     setTickets(tickets);
    //     //     // settickets(3);
    //     // }
        
        
    //     while(1);
    // }

    while(1);
    return 0;
}