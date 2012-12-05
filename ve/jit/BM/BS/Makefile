CC = gcc
CPP = g++
FLAGS = -O2 -lm
BOOST = /opt/local/include/ 

all: BlackScholes

bench: all
	./BlackScholes.sh

clean:
	rm -f BlackScholes *~

BlackScholes: BlackScholes.c
	$(CC) BlackScholes.c -o BlackScholes $(FLAGS)

