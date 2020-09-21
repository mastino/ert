/*
 * Simple Stencil example
 * Main program example
 *
 * Brian J Gravelle
 * gravelle@cs.uoregon.edu
 *
 */


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

//KERNEL

#define TRUE  1
#define FALSE 0

void kernel(double *__restrict__ data, 
            size_t nsize, 
            size_t ntrials,
            size_t *flops, 
            size_t *bytes_per_elem, 
            size_t *mem_accesses_per_elem);

struct Inputs {
  size_t  num_thr;
  size_t  nsize;
  size_t  nreps;
  char    print_info;
};

void get_input(int argc, char **argv, struct Inputs* input);


// main function
int main(int argc, char **argv) {

#ifdef USE_CALI
cali_id_t thread_attr = cali_create_attribute("thread_id", CALI_TYPE_INT, CALI_ATTR_ASVALUE | CALI_ATTR_SKIP_EVENTS);
#pragma omp parallel
{
cali_set_int(thread_attr, omp_get_thread_num());
}
#endif

#ifdef USE_LIKWID
LIKWID_MARKER_INIT;
#endif

  int err = FALSE;

  double wall_tot_start, wall_tot_end;
  double wall_init_start, wall_init_end;
  double wall_comp_start, wall_comp_end;
  double wall_free_start, wall_free_end;

  struct Inputs input;
  size_t ntrials = 1;
  size_t flops = 0;
  size_t bytes_per_elem = 0;
  size_t mem_accesses_per_elem = 0;
  double** data;
  double ai = 0.0;
  double gflops = 0.0;

  get_input(argc, argv, &input);

  omp_set_num_threads(input.num_thr);

  wall_tot_start = wall_init_start = omp_get_wtime();
  data = (double **)aligned_alloc(64, input.num_thr*sizeof(double*));
  for(size_t tid = 0; tid < input.num_thr; tid++)
    data[tid] = (double *)aligned_alloc(64, input.nsize*sizeof(double));
  if (input.print_info) printf("Running benchmark.......\n"); fflush(stdout);
  wall_comp_start = wall_init_end = omp_get_wtime();

  #pragma omp parallel
  {

#ifdef USE_CALI
CALI_MARK_BEGIN("kernel");
#endif
    
    size_t tid = omp_get_thread_num();
  
    kernel(data[tid], 
           input.nsize, 
           input.nreps,
           &flops,
           &bytes_per_elem, 
           &mem_accesses_per_elem);
    
#ifdef USE_CALI
CALI_MARK_END("kernel");
#endif
  }
  wall_free_start = wall_comp_end = omp_get_wtime();

  free(data);
  wall_tot_end = wall_free_end = omp_get_wtime();

  ai = ((double)flops)/((double)bytes_per_elem*(double)mem_accesses_per_elem);
  gflops = (flops*input.num_thr*input.nsize*input.nreps)/(wall_comp_end - wall_comp_start);

  if (input.print_info) {

    printf("\nRan new Shingle with \n\
      doubles per thr = %d \n\
      flops           = %d \n\
      Arith Intensity = %f \n\
      rep count       = %d \n\
      thread count    = %d \n",
      input.nsize, flops, ai, input.nreps, input.num_thr);
    printf("init time: %fs\n", (wall_init_end - wall_init_start));
    printf("free time: %fs\n", (wall_free_end - wall_free_start));
    printf("\ncomputation time: %fs\n", (wall_comp_end - wall_comp_start));
    printf("computation rate: %e GFLOP/s\n", gflops);

  } else {
    printf("%e\n", gflops);
  }

#ifdef USE_LIKWID
LIKWID_MARKER_CLOSE;
#endif

  return err;
}


void get_input(int argc, char **argv, struct Inputs* input) {

  int i = 1;

  input->num_thr     = 4;
  input->nsize       = 308224; // 2465792 bytes (308224 doubles) per core on L2 and 3 skylake
  input->nreps       = 5;
  input->print_info  = TRUE;

  for(i = 1; i < argc; i++) {

    if ( !(strcmp("-h", argv[i])) || !(strcmp("--help", argv[i])) ) {
      printf("shingles options:\n");
      printf("--threads [] -t [] .......... for number of threads\n");
      printf("--size [] -s [] ............. for number of doubles to use per thread\n");
      printf("--repititions [] -r [] ...... for number of repititions to perform\n");
      printf("--quiet -q .................. to remove output other than computation time\n");
      exit(1);
    }

    if ( !(strcmp("-t", argv[i])) || !(strcmp("--threads", argv[i])) ) {
      if (++i < argc){
        input->num_thr = atoi(argv[i]);
      } else {
        printf("Please include a thread count with that option\n");
        exit(1);
      }

    }

    if ( !(strcmp("-s", argv[i])) || !(strcmp("--size", argv[i])) ) {
      if (++i < argc){
        input->nsize = atoi(argv[i]);
      } else {
        printf("Please include a number of doubles with that option\n");
        exit(1);
      }

    }

    if ( !(strcmp("-r", argv[i])) || !(strcmp("--repititions", argv[i])) ) {
      if (++i < argc){
        input->nreps = atoi(argv[i]);
      } else {
        printf("Please include a number of reps with that option\n");
        exit(1);
      }

    }

    if ( !(strcmp("-q", argv[i])) || !(strcmp("--quiet", argv[i])) ) {
      input->print_info = FALSE;
    }

  }

}


