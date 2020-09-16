'''
shingle_defs.py

author Brian Gravelle
email  gravelle@lanl.gov

This  script defines a variety of strings that get put together into a shingle.h
  file that defines the kernel to run.

TODO redo as a static class

'''

import sys
import argparse

header = \
'''
#ifndef __SHINGLE_H__
#define __SHINGLE_H__

#ifdef USE_CALI
#include <caliper/cali.h>
#endif

#ifdef USE_LIKWID
#include <likwid-marker.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>


'''


function_def = \
'''
void kernel(double *__restrict__ data, 
            size_t nsize, 
            size_t ntrials,
            int *flops, 
            int *bytes_per_elem, 
            int *mem_accesses_per_elem) {


'''

variable_defs = \
'''
  *flops = %d + 1;
  *bytes_per_elem = sizeof(*data);
  *mem_accesses_per_elem = 2; // theoretically one load and one store

#ifdef ERT_INTEL
  __assume_aligned(data, 64);
#elif __xlC__
  __alignx(64, data);
#endif

  double alpha, const_beta;
  alpha      = 2.0;
  const_beta = 1.0;

'''


loop_start = \
'''
  size_t i, j;
  for (j = 0; j < ntrials; ++j) {
    #pragma unroll(8)
    for (i = 0; i < nsize; ++i) {

      double beta = const_beta;

'''

kernel_1 = '      beta = data[i] + alpha;\n'
kernel_2 = '      beta = beta * data[i] + alpha;\n'


loops_close = \
'''
      data[i] = -beta;
    }
  }
}
#endif


'''


#### compile line options

cc = "gcc -g -std=c11 "
# a64_opt = "-O3 -march=armv8.2-a+sve "
a64_opt = "-O3 "
sky_opt = "-O3 -march=skylake-avx512 "

opt = " "
opt += sky_opt

c_files = "shingles.c "
options = "-fopenmp "

h_file_opt = "-DKERNEL=\"#include %s\" "

exe_cmd = "./shingles_flops_%d.exe -s %d -t %d -r 100 -q"




