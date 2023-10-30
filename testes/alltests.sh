#!/bin/bash

function testaDezVezes() {
    for i in {1..10}
    do
        ./testes/teste.sh $1
    done
}

echo "Servidor com 0 clientes----------------"
testaDezVezes 0

echo "Servidor com 10 clientes---------------"
testaDezVezes 10

echo "Servidor com 100 clientes--------------"
testaDezVezes 100