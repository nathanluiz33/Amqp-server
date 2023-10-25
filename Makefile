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
	gcc -c hardcoded_values.c
	gcc -c amqp_client.c
	gcc -c packages.c
	gcc -c round_robin.c
	gcc -c amqp_queues.c
	gcc -o redes-servidor-exemplo-ep1 redes-servidor-exemplo-ep1.c *.o

clean:
	rm *.o
	rm test_amqp_queues
	rm test_round_robin
	rm redes-servidor-exemplo-ep1

teste_EP:
	amqp-declare-queue -q Hello
	amqp-declare-queue -q Hello1
	amqp-declare-queue -q Hello2

	amqp-publish -r Hello -b "Hello World"
	amqp-publish -r Hello1 -b "Hello World1"
	
	# amqp-consume -q Hello
	# amqp-consume -q Hello1
	# amqp-consume -q Hello2
	# amqp-consume -q Hello2

	# amqp-publish -r Hello -b "Hello World"
	# amqp-publish -r Hello1 -b "Hello World1"
	# amqp-publish -r Hello2 -b "Hello World2"
	# amqp-publish -r Hello2 -b "Hello World2"
	# amqp-publish -r Hello2 -b "Hello World2"

teste_consume:
	amqp-declare-queue -q Hello
	amqp-publish -r Hello -b "Hello World"

	amqp-consume -q Hello cat

%: %.c
	gcc -o $* $< amqp_queues.c

