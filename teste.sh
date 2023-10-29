for i in $(echo {0..100}); do amqp-declare-queue -q fila1; done
for i in $(echo {0..100}); do amqp-publish -r fila1 -b $i$'\n'; done
# for i in $(echo {0..1}); do amqp-consume -q fila1 cat & done
