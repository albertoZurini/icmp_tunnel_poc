#!/bin/sh
set -e
gcc -c -o icmp_utils.o icmp_utils.c
gcc -o receive receive.c icmp_utils.o
gcc -o send send.c icmp_utils.o
