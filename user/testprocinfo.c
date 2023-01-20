#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"


void printPstat(struct pstat *s){
    printf("Only used processes are shown\nPID|In Use|Original Tickets|Current Tickets|Time Slice\n");
    for(int i = 0; i < NPROC; ++i){
        if(s->inuse[i] != 0)
        printf("%d\t|%d\t|%d\t|%d\t|%d\n",s->pid[i], 
            s->inuse[i], s->tickets_original[i], s->tickets_current[i], s->time_slices[i]);
    }
}




int main(){

    settickets(10000); // so that testprocinfo gets scheduled quickly
    struct pstat stat;
    getpinfo(&stat);
    printPstat(&stat);

    return 0;
}