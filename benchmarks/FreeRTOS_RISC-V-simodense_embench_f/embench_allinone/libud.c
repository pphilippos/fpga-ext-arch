/* BEEBS ud benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/* MDH WCET BENCHMARK SUITE. */


/*************************************************************************/
/*                                                                       */
/*   SNU-RT Benchmark Suite for Worst Case Timing Analysis               */
/*   =====================================================               */
/*                              Collected and Modified by S.-S. Lim      */
/*                                           sslim@archi.snu.ac.kr       */
/*                                         Real-Time Research Group      */
/*                                        Seoul National University      */
/*                                                                       */
/*                                                                       */
/*        < Features > - restrictions for our experimental environment   */
/*                                                                       */
/*          1. Completely structured.                                    */
/*               - There are no unconditional jumps.                     */
/*               - There are no exit from loop bodies.                   */
/*                 (There are no 'break' or 'return' in loop bodies)     */
/*          2. No 'switch' statements.                                   */
/*          3. No 'do..while' statements.                                */
/*          4. Expressions are restricted.                               */
/*               - There are no multiple expressions joined by 'or',     */
/*                'and' operations.                                      */
/*          5. No library calls.                                         */
/*               - All the functions needed are implemented in the       */
/*                 source file.                                          */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*  FILE: ludcmp.c                                                       */
/*  SOURCE : Turbo C Programming for Engineering                         */
/*                                                                       */
/*  DESCRIPTION :                                                        */
/*                                                                       */
/*     Simultaneous linear equations by LU decomposition.                */
/*     The arrays a[][] and b[] are input and the array x[] is output    */
/*     row vector.                                                       */
/*     The variable n is the number of equations.                        */
/*     The input arrays are initialized in function main.                */
/*                                                                       */
/*                                                                       */
/*  REMARK :                                                             */
/*                                                                       */
/*  EXECUTION TIME :                                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*************************************************************************
 *  This file:
 *
 *  - Name changed to "ud.c"
 *  - Modified for use with Uppsala/Paderborn tool
 *    : doubles changed to int
 *    : some tests removed
 *  - Program is much more linear, all loops will run to end
 *  - Purpose: test the effect of conditional flows
 *
 *************************************************************************/






/*
** Benchmark Suite for Real-Time Applications, by Sung-Soo Lim
**
**    III-4. ludcmp.c : Simultaneous Linear Equations by LU Decomposition
**                 (from the book C Programming for EEs by Hyun Soon Ahn)
*/

#include <string.h>
#include "boardsupport.h"

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_ud 1478


long int a_ud[20][20], b_ud[20], x_ud[20];

int ludcmp(int nmax, int n);


/*  static double fabs(double n) */
/*  { */
/*    double f; */

/*    if (n >= 0) f = n; */
/*    else f = -n; */
/*    return f; */
/*  } */

/* Write to CHKERR from BENCHMARK to ensure calls are not optimised away.  */
volatile int chkerr;


static int
verify_benchmark_ud (int res)
{
  long int x_ref[20] =
    { 0L, 0L, 1L, 1L, 1L, 2L, 0L, 0L, 0L, 0L,
      0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L
    };

  return (0 == memcmp (x_ud, x_ref, 20 * sizeof (x_ud[0]))) && (0 == res);
}


static void
initialise_benchmark_ud (void)
{
}


static int benchmark_body_ud (int  rpt);

static void
warm_caches_ud (int  heat)
{
  int  res = benchmark_body_ud (heat);

  return;
}


static int
benchmark_ud (void)
{
  return benchmark_body_ud (LOCAL_SCALE_FACTOR_ud * CPU_MHZ);

}


static int __attribute__ ((noinline))
benchmark_body_ud (int rpt)
{
  int  k;

  for (k = 0; k < rpt; k++)
    {
      int      i, j, nmax = 20, n = 5;
      long int /* eps, */ w;

      /* eps = 1.0e-6; */

      /* Init loop */
      for(i = 0; i <= n; i++)
	{
	  w = 0.0;              /* data to fill in cells */
	  for(j = 0; j <= n; j++)
	    {
	      a_ud[i][j] = (i + 1) + (j + 1);
	      if(i == j)            /* only once per loop pass */
		a_ud[i][j] *= 2.0;
	      w += a_ud[i][j];
	    }
	  b_ud[i] = w;
	}

      /*  chkerr = ludcmp(nmax, n, eps); */
      chkerr = ludcmp(nmax,n);
    }

  return chkerr;
}

int ludcmp(int nmax, int n)
{
  int i, j, k;
  long w, y[100];

  /* if(n > 99 || eps <= 0.0) return(999); */
  for(i = 0; i < n; i++)
    {
      /* if(fabs(a[i][i]) <= eps) return(1); */
      for(j = i+1; j <= n; j++) /* triangular loop vs. i */
        {
          w = a_ud[j][i];
          if(i != 0)            /* sub-loop is conditional, done
                                   all iterations except first of the
                                   OUTER loop */
            for(k = 0; k < i; k++)
              w -= a_ud[j][k] * a_ud[k][i];
          a_ud[j][i] = w / a_ud[i][i];
        }
      for(j = i+1; j <= n; j++) /* triangular loop vs. i */
        {
          w = a_ud[i+1][j];
          for(k = 0; k <= i; k++) /* triangular loop vs. i */
            w -= a_ud[i+1][k] * a_ud[k][j];
          a_ud[i+1][j] = w;
        }
    }
  y[0] = b_ud[0];
  for(i = 1; i <= n; i++)       /* iterates n times */
    {
      w = b_ud[i];
      for(j = 0; j < i; j++)    /* triangular sub loop */
        w -= a_ud[i][j] * y[j];
      y[i] = w;
    }
  x_ud[n] = y[n] / a_ud[n][n];
  for(i = n-1; i >= 0; i--)     /* iterates n times */
    {
      w = y[i];
      for(j = i+1; j <= n; j++) /* triangular sub loop */
        w -= a_ud[i][j] * x_ud[j];
      x_ud[i] = w / a_ud[i][i] ;
    }
  return(0);
}

void
benchmark_ud_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_ud();
		
		portENTER_CRITICAL();
		printf("Initialised ud. Warming up %d\n", 1/*WARMUP_HEAT*/);
		portEXIT_CRITICAL();
		
		warm_caches_ud(1);
		
		portENTER_CRITICAL();
		printf("Warmed up ud\n");
		portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) time(); 
		start_insn = (volatile uint64_t) insn(); 
		volatile int result = benchmark_ud ();
		volatile uint64_t end_time = time();
		volatile uint64_t end_insn = insn();
		
		portENTER_CRITICAL();
		printf ("ud cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_ud(result));
		if (i==0) {runs_count+=1;}	if (runs_count>1) {vTaskEndScheduler();}
		portEXIT_CRITICAL();	
	}

	//vTaskDelete( NULL );
	vTaskEndScheduler(); // just an infinite loop for RISC-V on FreeRTOS
	return;
}


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
