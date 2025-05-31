#!/bin/bash

gcc client.c -o client
gcc server.c -o server
./server &
sleep 1
touch result.txt
echo "First task:" > result.txt
./task1.sh
echo "" >> result.txt
echo "Second task:" >> result.txt

echo "Run 100 clients twice" >> result.txt

./task1.sh
./task1.sh

echo "Check descriptor num and sbrk()" >> result.txt
echo "First accept:" >> result.txt
cat /tmp/server.log | grep "Accept" | head -n 1 >> result.txt

echo "Last two accepts:" >> result.txt
cat /tmp/server.log | grep "Accept" | tail -n 2 >> result.txt
echo "" >> result.txt
echo "Third task:" >> result.txt
./task3.sh
rm /tmp/serv*
