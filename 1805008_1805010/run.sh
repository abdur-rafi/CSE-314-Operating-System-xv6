#!/bin/bash

# rm santa
# gcc santa.c ../zemaphore/zemaphore.c -o santa -lpthread
# ./santa

rm santa
gcc santa-cv.c -o santa -lpthread 
./santa 