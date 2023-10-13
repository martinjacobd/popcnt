# mostly cribbed from https://makefiletutorial.com/

TARGET := popcnt

SRCS := popcnt.c

CC = gcc
CFLAGS = -O3 -Wall -pedantic

run: popcnt data.bin
	./popcnt

popcnt: popcnt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) popcnt.c -o $@ $(LDFLAGS)

data.bin: gen_data
	./gen_data

gen_data: gen_data.c
	$(CC) $(CPPFLAGS) $(CFLAGS) gen_data.c -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f popcnt gen_data data.bin

