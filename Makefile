CC = gcc

all: mysh

mysh:
	$(CC) src/mysh.c -o mysh

tests:
	$(CC) test/hello.c -o hello
	$(CC) test/hello2.c -o hello2
	$(CC) test/kyle.c -o kyle
	$(CC) test/redirecttest.c -o redirecttest
	$(CC) test/pipetest.c -o pipetest

