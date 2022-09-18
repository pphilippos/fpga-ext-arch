/* BEEBS cubic benchmark

   Contributor: James Pallister <james.pallister@bristol.ac.uk>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#include <string.h>
#include <math.h>
//#include "support.h"
//#include "boardsupport.h"
#include "snipmath.h"
#include "libcubic.c"

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_cubic 10




static int soln_cnt0;
static int soln_cnt1;
static float res0[3];
static float res1;


static int
verify_benchmark_cubic (int res __attribute ((unused)) )
{
  static const float exp_res0[3] = {2.0, 6.0, 2.5};
  const float exp_res1 = 2.5;
  return (3 == soln_cnt0)
    && float_eq_beebs(exp_res0[0], res0[0])
    && float_eq_beebs(exp_res0[1], res0[1])
    && float_eq_beebs(exp_res0[2], res0[2])
    && (1 == soln_cnt1)
    && float_eq_beebs(exp_res1, res1);
}


static void
initialise_benchmark_cubic (void)
{
}


static int benchmark_body_cubic (int  rpt);

static void
warm_caches_cubic (int  heat)
{
  int  res = benchmark_body_cubic (heat);

  return;
}


static int
benchmark_cubic (void)
{
  return benchmark_body_cubic (LOCAL_SCALE_FACTOR_cubic * CPU_MHZ);
}


static int
benchmark_body_cubic (int rpt)
{
  int  i;

  for (i = 0; i < rpt; i++)
    {
      float  a1 = 1.0, b1 = -10.5, c1 = 32.0, d1 = -30.0;
      float  a2 = 1.0, b2 = -4.5, c2 = 17.0, d2 = -30.0;
      float  a3 = 1.0, b3 = -3.5, c3 = 22.0, d3 = -31.0;
      float  a4 = 1.0, b4 = -13.7, c4 = 1.0, d4 = -35.0;
      int     solutions;

      float output[48] = {0};
      float *output_pos = &(output[0]);

      /* solve some cubic functions */
      /* should get 3 solutions: 2, 6 & 2.5   */
      SolveCubic(a1, b1, c1, d1, &solutions, output);
      soln_cnt0 = solutions;
      memcpy(res0,output,3*sizeof(res0[0]));
      /* should get 1 solution: 2.5           */
      SolveCubic(a2, b2, c2, d2, &solutions, output);
      soln_cnt1 = solutions;
      res1 = output[0];
      SolveCubic(a3, b3, c3, d3, &solutions, output);
      SolveCubic(a4, b4, c4, d4, &solutions, output);
      /* Now solve some random equations */
      for(a1=1;a1<3;a1++) {
	for(b1=10;b1>8;b1--) {
	  for(c1=5;c1<6;c1+=0.5) {
            for(d1=-1;d1>-3;d1--) {
	      SolveCubic(a1, b1, c1, d1, &solutions, output_pos);
            }
	  }
	}
      }
    }

   return 0;
}

void
benchmark_cubic_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_cubic();
		
		//portENTER_CRITICAL();
		printf("Initialised cubic. Warming up %d\n", 1/*WARMUP_HEAT*/);
		//portEXIT_CRITICAL();
		
		warm_caches_cubic(1);
		
		//portENTER_CRITICAL();
		printf("Warmed up cubic\n");
		//portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) 0; 
		start_insn = (volatile uint64_t) 0; 
		volatile int result = benchmark_cubic ();
		volatile uint64_t end_time = 0;
		volatile uint64_t end_insn = 0;
		
		//portENTER_CRITICAL();
		printf ("cubic cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_cubic(result));
		if (i==0) {runs_count+=1;}	if (runs_count==0) return;//{//vTaskEndScheduler();} 
		//portEXIT_CRITICAL();	
	}

	//vTaskDelete( NULL );
	//vTaskEndScheduler(); // just an infinite loop for RISC-V on FreeRTOS
	return;
}

/* vim: set ts=3 sw=3 et: */
