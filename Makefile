round_robin:
	gcc -c amqp_client.c
	gcc -c round_robin.c
	gcc -o test_round_robin test_round_robin.c *.o
	./test_round_robin < in.txt

amqp_queues:
	gcc -c amqp_client.c
	gcc -c round_robin.c
	gcc -c amqp_queues.c
	gcc -o test_amqp_queues test_amqp_queues.c *.o
	./test_amqp_queues

run:
	gcc -c amqp_client.c
	gcc -c round_robin.c
	gcc -c amqp_queues.c
	gcc -o redes-servidor-exemplo-ep1 redes-servidor-exemplo-ep1.c *.o

clean:
	rm *.o
	rm test_amqp_queues
	rm test_round_robin
	rm redes-servidor-exemplo-ep1

%: %.c
	gcc -o $* $< amqp_queues.c

