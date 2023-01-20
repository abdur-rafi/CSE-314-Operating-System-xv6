#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"


void printPstat(struct pstat *s){
    printf("PID  | In Use | Original Tickets | Current Tickets | Time Slice\n");
    for(int i = 0; i < NPROC; ++i){
    printf("%d    | %d      |%d              |%d               |%d\n",s->pid[i], 
        s->inuse[i], s->tickets_original[i], s->tickets_current[i], s->time_slices[i]);
    }
}

int main(){
    struct pstat stat;

    settickets(100);
    getpinfo(&stat);
    printPstat(&stat);
    // random();

    return 0;

}