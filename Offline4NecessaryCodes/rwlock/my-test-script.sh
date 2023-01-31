#!/bin/bash
g++ -w test-reader-pref.cpp rwlock-reader-pref.cpp -o rwlock-reader-pref -lpthread
g++ -w test-writer-pref.cpp rwlock-writer-pref.cpp -o rwlock-writer-pref -lpthread

echo "TESTSET: Running Testcases with reader preference"
correct=0
total=0
for i in {0..20..1}
do
    for j in {0..20..1}
    do
        echo "Running Testcases with $i reader and $j writer"
        out=`./rwlock-reader-pref $i $j`
        echo $out
        if [ "$out" = "PASSED" ]; then
            correct=$((correct+1))
        fi
        total=$((total+1))
    done
done
echo "Test Cases Passed: $correct"
echo "Test Cases Total: $total"

echo "TESTSET: Running Testcases with writer preference"
correct=0
total=0
for i in {0..20..1}
do
    for j in {0..20..1}
    do
        echo "Running Testcases with $i reader and $j writer"
        out=`./rwlock-writer-pref $i $j`
        echo $out
        if [ "$out" = "PASSED" ]; then
            correct=$((correct+1))
        fi
        total=$((total+1))
    done
done
echo "Test Cases Passed: $correct"
echo "Test Cases Total: $total"
