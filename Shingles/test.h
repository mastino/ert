
#ifndef __SHINGLE_H__
#define __SHINGLE_H__



void kernel(double *__restrict__ data, 
            size_t nsize, 
            size_t ntrials,
            int *bytes_per_elem, 
            int *mem_accesses_per_elem) {



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


  size_t i, j;
  for (j = 0; j < ntrials; ++j) {
    #pragma unroll(8)
    for (i = 0; i < nsize; ++i) {

      double beta = const_beta;

      beta = beta * data[i] + alpha;
      beta = beta * data[i] + alpha;
      beta = beta * data[i] + alpha;
      beta = beta * data[i] + alpha;
      beta = data[i] + alpha;

      data[i] = -beta;
    }
  }
}
#endif



