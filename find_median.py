#!/usr/bin/python
import numpy
import sys

with open(sys.argv[1]) as f:
    numbers = []
    for line in f:
        if(line != ""):
            numbers.append(float(line))
    numbers.sort()

if(len(numbers) > 0):
	median = numbers[len(numbers)/2]
	print("%f %d\n" %(median, len(numbers)))
