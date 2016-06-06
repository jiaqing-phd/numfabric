#!/usr/bin/python
import numpy
import sys

with open(sys.argv[1]) as f:
    numbers = []
    for line in f:
        if(line != "" and float(line)> 0.0):
            numbers.append(float(line))
    numbers.sort()

if(len(numbers) > 0):
	median = numbers[len(numbers)/2]
	print("%f %d" %(median, len(numbers)))
