#!/usr/bin/env python3

current_filename = ''
current_counter = 0
current_denom = 0

def output():
	if current_filename:
		print(current_filename, current_counter/current_denom)

for line in open('output_own_2'):
	splitted = line.split(' ')
	if len(splitted) == 1:
		output()
		current_filename = line.rstrip('\n')
		current_counter = 0
		current_denom = 0
	else:
		#time_in_s = float(splitted[4])
		time_in_s = float(splitted[1][:-3]) / 1000 / 1000 # µs to s
		current_counter += time_in_s
		current_denom += 1

output()
