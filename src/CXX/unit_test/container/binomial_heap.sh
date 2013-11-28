#!/bin/sh

./randnums 1000000 | ./binomial_heap >/dev/null
