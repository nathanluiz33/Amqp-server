build:
	gcc -c hardcoded_values.c
	gcc -c amqp_client.c
	gcc -c packages.c
	gcc -c round_robin.c
	gcc -c amqp_queues.c
	gcc -o redes-servidor-ep1 redes-servidor-ep1.c *.o

run:
	./redes-servidor-ep1 5672

clean:
	rm *.o
	rm redes-servidor-ep1

run_tests:
	./testes/alltests.sh

%: %.c
	gcc -o $* $< amqp_queues.c