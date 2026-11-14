CC      ?= cc
CFLAGS  ?= -O3

primes: main.c
	$(CC) $(CFLAGS) -o primes main.c

clean:
	rm -f primes

.PHONY: clean
