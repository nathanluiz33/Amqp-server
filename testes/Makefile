round_robin:
	gcc -c amqp_client.c
	gcc -c round_robin.c
	gcc -o test_round_robin test_round_robin.c *.o
	./test_round_robin

amqp_queues:
	gcc -c amqp_client.c
	gcc -c round_robin.c
	gcc -c amqp_queues.c
	gcc -o test_amqp_queues test_amqp_queues.c *.o
	./test_amqp_queues