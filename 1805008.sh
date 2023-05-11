#!/bin/bash

maxMark=100
maxStudentId=5

if (($# != 0)); then
    maxMark=$1
fi
if (($# > 1)); then
    maxStudentId=$2
fi

>output.csv
cd Offline_1_files/Submissions

ids=()
for (( i=1;i<=maxStudentId;i++ ))
do
    f="180512$i"
    if [[ ! -f ./$f/$f.sh ]]; then
        echo "$f,0" >> ./../../output.csv
        continue
    fi
    ids+=($f)
done

for f in ${ids[@]}
do
    mark=maxMark
    deduction=$(( 5 * $(bash ./$f/$f.sh | diff -w - ../AcceptedOutput.txt | grep '^[<>]' - | wc -l) ))
    mark=$((mark - deduction))
    if (( mark < 0)); then
        mark=0
    fi
    for f2 in ${ids[@]}
    do
        if [[ $f = $f2 ]] ; then
            continue
        fi
        if (( $(diff -BZ ./$f/$f.sh ./$f2/$f2.sh | grep '^[<>]' - | wc -l) == 0 )); then
            mark=$((-maxMark))
            break
        fi
    done
    echo "$f,$mark" >> ./../../output.csv
done
cd ./../../
sort  -o output.csv output.csv 
echo "student_id, score" | cat - output.csv > temp && mv temp output.csv
