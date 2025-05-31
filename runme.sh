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
clients=(10 20 40 60 80 100)
delays=(0 0.2 0.4 0.6 0.8 1)
for c in ${clients[@]}
do
    for d in ${delays[@]}
    do
        echo "Test clients: $c delay: $d" >> result.txt
        touch /tmp/client.log
        SECONDS=0
        for ((i=1; i<=$c;i++));
        do
            (./client $i 30 $d < nums) >> /tmp/client.log &

        done
        wait
        client_time=$(cat /tmp/client.log | grep "client time:" | grep -Eo '[0-9]+' | sort -rn | head -n 1)
        echo "Client time: $client_time" >> result.txt
        duration=$SECONDS
        effective_time=$((duration - client_time))
        echo "Total time: $duration" >> result.txt
        echo "Efficiency: $effective_time" >> result.txt
        rm /tmp/client.log
    done
done
rm /tmp/serv*
