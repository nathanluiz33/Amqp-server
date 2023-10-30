#!/bin/bash
make build > /dev/null

clientes=$1
echo $clientes

./redes-servidor-ep1 5672 > /dev/null &

pidServer=$!

tcpdump -i lo0 -s 0 'tcp port amqp' > data_bytes.txt &
pidTshark=$!

pid_list=()

function declareQueue() {
    for ((i = 1; i<=$1; i++))
    do
        remainder=$((i % 2))
        amqp-declare-queue -q "queue$remainder" > /dev/null
        # sleep 0.1
    done
}
function publish() {
    for ((i = 1; i<=$1; i++))
    do
        remainder=$((i % 2))
        amqp-publish -r "queue$remainder" -b teste > /dev/null
        # sleep 0.1
    done
}
function consume() {
    for ((i = 1; i<=$1; i++))
    do
        remainder=$((i % 2))
        amqp-consume -q "queue$remainder" cat > /dev/null &
        sleep 0.1
        pid_list+=($!)
    done
}

function getCPUUsage() {
    while kill -0 $pidServer 2>/dev/null
    do 
        top -n 1 -l 1 | grep "CPU usage" | cut -d "," -f 1 | cut -d " " -f 3 | cut -d "%" -f 1 | tr , . >> cpu_usage.txt
        sleep 0.1
    done
}

getCPUUsage &
pidCPU=$!

echo "Declarando..."
declareQueue $clientes

echo "Publicando..."
publish $clientes

echo "Consumindo..."
consume $clientes

echo "Publicando..."
publish $clientes

sleep 1
kill $pidCPU
sleep 4
for value in "${pid_list[@]}"
do
    kill $value
done

kill $pidServer
kill $pidTshark
python3 testes/average.py

make clean
rm data_bytes.txt
rm cpu_usage.txt