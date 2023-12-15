CC = gcc
CFLAGS = -Wall -Wextra -I. -lwiringPi -lcurl -pthread

all: testmain

testmain: test_thread.o sms.o
	$(CC) $^ -o $@ $(CFLAGS)

main.o: test_thread.c sms.h
	$(CC) -c $< -o $@ $(CFLAGS)

sms.o: sms.c sms.h
	$(CC) -c $< -o $@ $(CFLAGS)

