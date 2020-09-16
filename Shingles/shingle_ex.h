#ifndef __SHINGLE_H__
#define __SHINGLE_H__

#define KERNEL1(a, b, c) ((a) = (b) + (c))
#define KERNEL2(a, b, c) ((a) = (a) * (b) + (c))



void kernel(double *__restrict__ data, 
            size_t nsize, 
            size_t ntrials, 
            int *bytes_per_elem, 
            int *mem_accesses_per_elem) {

  *bytes_per_elem = sizeof(*data);
  *mem_accesses_per_elem = 2; // theoretically one load and one store

#ifdef ERT_INTEL
  __assume_aligned(data, ERT_ALIGN);
#elif __xlC__
  __alignx(ERT_ALIGN, data);
#endif

  // data needs to be initilized to -1 coming in
  // datand with alpha=2 and beta=1, data=-1 is preserved upon return
  double alpha, const_beta;
  alpha      = 2.0;
  const_beta = 1.0;

  size_t i, j;
  for (j = 0; j < ntrials; ++j) {
#pragma unroll(8)
    for (i = 0; i < nsize; ++i) {
      double beta = const_beta;
#if (ERT_FLOP & 1) == 1 /* add 1 flop */
      KERNEL1(beta, data[i], alpha);
#endif
#if (ERT_FLOP & 2) == 2 /* add 2 flops */
      KERNEL2(beta, data[i], alpha);
#endif
#if (ERT_FLOP & 4) == 4 /* add 4 flops */
      REP2(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 8) == 8 /* add 8 flops */
      REP4(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 16) == 16 /* add 16 flops */
      REP8(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 32) == 32 /* add 32 flops */
      REP16(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 64) == 64 /* add 64 flops */
      REP32(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 128) == 128 /* add 128 flops */
      REP64(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 256) == 256 /* add 256 flops */
      REP128(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 512) == 512 /* add 512 flops */
      REP256(KERNEL2(beta, data[i], alpha));
#endif
#if (ERT_FLOP & 1024) == 1024 /* add 1024 flops */
      REP512(KERNEL2(beta, data[i], alpha));
#endif

      data[i] = -beta;
    }
  }
}

#endif

