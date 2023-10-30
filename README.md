# Amqp-server

Este é um projeto de MAC0352 de 2023.2. O objetivo é construir um servidor AMQP
que suporta os seguintes métodos

- amqp-declare-queue -q [queue-name]
- amqp-publish -r [queue-name]
- amqp-consume -q [queue-name] cat

Para compilar os arquivos, basta rodar:

- make build

Para rodar somente o servidor:

- make run

Para rodar um script com alguns clientes:

- make run_tests

Para limpar o diretório:

- make clean