#!/bin/sh
# fastest-primes — run command.
# Runs the prebuilt ./primes binary if it executes; otherwise builds it
# from main.c with the system C compiler and runs that. No compile step
# on the timed path when the binary is present.
# Usage: echo 5000000 | ./run.sh
set -e
cd "$(dirname "$0")"
if [ ! -x ./primes ]; then
    cc -O3 -o primes main.c
fi
exec ./primes
