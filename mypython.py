#!/usr/bin/python
# Written by Adam Sunderman for CS 344
import random
import sys
import os

for i in range(1,4):
	buildup=""
	filename="file"+str(i)
	print("\nWriting file: " + filename)
	thefile=open(filename, 'w+t')
	if not os.access(filename, os.W_OK):
		print("Couldn't make file " + str(i))
		sys.exit()
	for j in range(1,11):
		buildup+=random.choice('abcdefghijklmnopqrstuvwxyz')
	print(filename.title() + " contents: " + buildup + "\n")
	thefile.write(str(buildup + "\n"))
	
var1 = random.randint(1,42)
var2 = random.randint(1,42)
print("Random number 1: " + str(var1) + "\n" + "Random number 2: " + str(var2) + "\n" + "Random product: " + str(var1 * var2) + "\n")



