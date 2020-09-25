'''
run_shingles.py

author Brian Gravelle
email  gravelle@lanl.gov

This  script generates and runs benchamrks to identify benchamrks that hit the roofline 
at different arithmentic intensties on a particular system.

shingles.c is the driver, this generates a series of shingles.h files with different AIs

'''

import sys
import argparse
from shingle_defs import *
from math import floor
from subprocess import check_output

def print_shingles_h(filename, flops):
  
	file = open(filename,"w")
	
	file.write(header)
	file.write(function_def)
	file.write(variable_defs%flops)
	file.write(loop_start)

	if (flops % 2) == 1:
		file.write(kernel_1)
		
	for i in range(0,int(floor(flops/2))):
		file.write(kernel_2)

	file.write(loops_close)
	
	file.write("\n")
	file.close()





if __name__ == '__main__':


	# parser = argparse.ArgumentParser(description='Process some numbers.')
	# parser.add_argument('-p', '--peak', dest='peak', action='store', 
	# 					required=True, type=float, 
	# 					help='peak GFLOPS')
	# parser.add_argument('-s1', '--L1_slope', dest='L1_slope', action='store', 
	# 					required=True, type=float, 
	# 					help='BW calculated for L1 cache')
	# parser.add_argument('-c1', '--L1_corner', dest='L1_corner', action='store', 
	# 					required=True, type=float, 
	# 					help='AI calculated for L1 cache max GFLOPS')
	# parser.add_argument('-s2', '--L2_slope', dest='L2_slope', action='store', 
	# 					required=True, type=float, 
	# 					help='BW calculated for L2 cache')
	# parser.add_argument('-c2', '--L2_corner', dest='L2_corner', action='store', 
	# 					required=True, type=float, 
	# 					help='AI calculated for L2 cache max GFLOPS')
	# parser.add_argument('-se', '--ext_mem_slope', dest='em_slope', action='store', 
	# 					required=True, type=float, 
	# 					help='BW calculated for external memory')
	# parser.add_argument('-ce', '--ext_mem_corner', dest='em_corner', action='store', 
	# 					required=True, type=float, 
	# 					help='AI calculated for external memory max GFLOPS')
	# parser.add_argument('-f', '--file', dest='file', action='store', 
	# 					required=False, default="roofline.csv", 
	# 					help='AI calculated for external memory max GFLOPS')

	# args = parser.parse_args()

	# print("")
	# print("Peak GFLOPS    = %f" % args.peak)
	# print("")
	# print("L1 slope       = %f" % args.L1_slope)
	# print("L1 max AI      = %f" % args.L1_corner)
	# print("L1 y-intercept = %f" % (args.peak - args.L1_slope*args.L1_corner) )
	# print("")
	# print("L2 slope       = %f" % args.L2_slope)
	# print("L2 max AI      = %f" % args.L2_corner)
	# print("L2 y-intercept = %f" % (args.peak - args.L2_slope*args.L2_corner) )
	# print("")
	# print("EM slope       = %f" % args.em_slope)
	# print("EM max AI      = %f" % args.em_corner)
	# print("EM y-intercept = %f" % (args.peak - args.em_slope*args.em_corner) )
	# print("")

	run = False
	num_shingles = 6
	AI_li = [0.0625, 0.125, 0.5, 1.0, 6.25, 12.5, 25.0]
	bytes_per_elem = 16 # DP load and DP store
	data_start_size = 10000

	kern_dir = "test_kern_files/"
	kern_pattern = "shingles_flops_%d.c"
	exe_pattern = "shingles_flops_%d.exe"

	for ai in AI_li:
		flops = int(ai*bytes_per_elem)
		# print kern_dir+(kern_pattern%flops)
		print_shingles_h(kern_dir+(kern_pattern%flops), flops)

		# make = cc + a64_opt + c_files + (h_file_opt%(h_dir+(h_pattern%flops))) + options
		make = cc + opt + c_files + kern_dir+(kern_pattern%flops) + " " + options + "-o " + (exe_pattern%flops)
		output=check_output(make, shell=True)
		if not output.strip() is "":
			print make
			print output
			quit()

		if run:
			nthrs = 112
			nreps = 10000
			for s in [4000, 16000, 64000, 256000, 512000, 1024000, 2048000]:
			# for s in [1024000, 2048000, 4096000, 8192000, 16384000, 32768000, 65536000, 131072000]:
				cmd = exe_cmd%(flops,s,nthrs,nreps)
				output=check_output(cmd, shell=True)
				if not output.strip() is "":
					print cmd
					print output



