#!/usr/bin/python
import numpy
import sys

nonconverge = 0
with open(sys.argv[1]) as f:
    numbers = []
    for line in f:
        if(line != "" and float(line)> 0.0 and float(line) < 0.1):
            numbers.append(float(line))
        if(line != "" and float(line) == 0.1):
            nonconverge += 1
    numbers.sort()

if(len(numbers) > 0):
	median = numbers[len(numbers)/2]
	#median = numpy.percentile(numbers, 95)
	print("%f %f %d %d" %(median, numpy.percentile(numbers, 95),len(numbers), nonconverge))
