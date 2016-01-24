#!/usr/bin/python
import numpy
import sys

with open(sys.argv[1]) as f:
    numbers = []
    for line in f:
        numbers.append(float(line))
    numbers.sort()

median = numbers[len(numbers)/2]
print("%f %d\n" %(median, len(numbers)))
