/* BEEBS minver benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor Pierre Langlois <pierre.langlois@embecosm.com>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   *************************************************************************
   *                                                                       *
   *   SNU-RT Benchmark Suite for Worst Case Timing Analysis               *
   *   =====================================================               *
   *                              Collected and Modified by S.-S. Lim      *
   *                                           sslim@archi.snu.ac.kr       *
   *                                         Real-Time Research Group      *
   *                                        Seoul National University      *
   *                                                                       *
   *                                                                       *
   *        < Features > - restrictions for our experimental environment   *
   *                                                                       *
   *          1. Completely structured.                                    *
   *               - There are no unconditional jumps.                     *
   *               - There are no exit from loop bodies.                   *
   *                 (There are no 'break' or 'return' in loop bodies)     *
   *          2. No 'switch' statements.                                   *
   *          3. No 'do..while' statements.                                *
   *          4. Expressions are restricted.                               *
   *               - There are no multiple expressions joined by 'or',     *
   *                'and' operations.                                      *
   *          5. No library calls.                                         *
   *               - All the functions needed are implemented in the       *
   *                 source file.                                          *
   *                                                                       *
   *                                                                       *
   *************************************************************************
   *                                                                       *
   *  FILE: minver.c                                                       *
   *  SOURCE : Turbo C Programming for Engineering by Hyun Soo Ahn         *
   *                                                                       *
   *  DESCRIPTION :                                                        *
   *                                                                       *
   *     Matrix inversion for 3x3 floating point matrix.                   *
   *                                                                       *
   *  REMARK :                                                             *
   *                                                                       *
   *  EXECUTION TIME :                                                     *
   *                                                                       *
   *                                                                       *
   *************************************************************************

*/

#include <math.h>
#include <string.h>
//#include "boardsupport.h"

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_minver 555

int minver (int row, int col, float eps);
int mmul (int row_a, int col_a, int row_b, int col_b);

static float a_ref[3][3] = {
  {3.0, -6.0, 7.0},
  {9.0, 0.0, -5.0},
  {5.0, -8.0, 6.0},
};

static float b_[3][3] = {
  {-3.0, 0.0, 2.0},
  {3.0, -2.0, 0.0},
  {0.0, 2.0, -3.0},
};

static float a_[3][3], c_[3][3], d_[3][3], det;

static float
minver_fabs (float n)
{
  float f;

  if (n >= 0)
    f = n;
  else
    f = -n;
  return f;
}

int
mmul (int row_a, int col_a, int row_b, int col_b)
{
  int i, j, k, row_c, col_c;
  float w;

  row_c = row_a;
  col_c = col_b;

  if (row_c < 1 || row_b < 1 || col_c < 1 || col_a != row_b)
    return (999);
  for (i = 0; i < row_c; i++)
    {
      for (j = 0; j < col_c; j++)
	{
	  w = 0.0;
	  for (k = 0; k < row_b; k++)
	    w += a_[i][k] * b_[k][j];
	  c_[i][j] = w;
	}
    }

  return (0);
}


int
minver (int row, int col, float eps)
{
  int work[500], i, j, k, r, iw, u, v;
  float w, wmax, pivot, api, w1;

  r = w = 0;
  if (row < 2 || row > 500 || eps <= 0.0)
    return (999);
  w1 = 1.0;
  for (i = 0; i < row; i++)
    work[i] = i;
  for (k = 0; k < row; k++)
    {
      wmax = 0.0;
      for (i = k; i < row; i++)
	{
	  w = minver_fabs (a_[i][k]);
	  if (w > wmax)
	    {
	      wmax = w;
	      r = i;
	    }
	}
      pivot = a_[r][k];
      api = minver_fabs (pivot);
      if (api <= eps)
	{
	  det = w1;
	  return (1);
	}
      w1 *= pivot;
      u = k * col;
      v = r * col;
      if (r != k)
	{
	  w1 = -w;
	  iw = work[k];
	  work[k] = work[r];
	  work[r] = iw;
	  for (j = 0; j < row; j++)
	    {
	      w = a_[k][j];
	      a_[k][j] = a_[r][j];
	      a_[r][j] = w;
	    }
	}
      for (i = 0; i < row; i++)
	a_[k][i] /= pivot;
      for (i = 0; i < row; i++)
	{
	  if (i != k)
	    {
	      v = i * col;
	      w = a_[i][k];
	      if (w != 0.0)
		{
		  for (j = 0; j < row; j++)
		    if (j != k)
		      a_[i][j] -= w * a_[k][j];
		  a_[i][k] = -w / pivot;
		}
	    }
	}
      a_[k][k] = 1.0 / pivot;
    }

  for (i = 0; i < row; i++)
    {
      while (1)
	{
	  k = work[i];
	  if (k == i)
	    break;
	  iw = work[k];
	  work[k] = work[i];
	  work[i] = iw;
	  for (j = 0; j < row; j++)
	    {
	      u = j * col;
	      w = a_[k][i];
	      a_[k][i] = a_[k][k];
	      a_[k][k] = w;
	    }
	}
    }

  det = w1;

  return (0);
}


static int
verify_benchmark_minver (int res __attribute ((unused)))
{
  int i, j;
  float eps = 1.0e-6;

  static float c_exp[3][3] = {
    {-27.0, 26.0, -15.0},
    {-27.0, -10.0, 33.0},
    {-39.0, 28.0, -8.0}
  };

  static float d_exp[3][3] = {
    {0.133333325, -0.199999958, 0.2666665910},
    {-0.519999862, 0.113333330, 0.5266665220},
    {0.479999840, -0.359999895, 0.0399999917}
  };

  /* Allow small errors in floating point */

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      if (float_neq_beebs(c_[i][j], c_exp[i][j]) || float_neq_beebs(d_[i][j], d_exp[i][j]))
	return 0;

  return float_eq_beebs(det, -16.6666718);
}


static void
initialise_benchmark_minver (void)
{
}


static int benchmark_body_minver (int  rpt);

static void
warm_caches_minver (int  heat)
{
  int  res = benchmark_body_minver (heat);

  return;
}


static int
benchmark_minver (void)
{
  return benchmark_body_minver (LOCAL_SCALE_FACTOR_minver * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body_minver (int rpt)
{
  int i;

  for (i = 0; i < rpt; i++)
    {
      float eps = 1.0e-6;

      memcpy (a_, a_ref, 3 * 3 * sizeof (a_[0][0]));
      minver (3, 3, eps);
      memcpy (d_, a_, 3 * 3 * sizeof (a_[0][0]));
      memcpy (a_, a_ref, 3 * 3 * sizeof (a_[0][0]));
      mmul (3, 3, 3, 3);
    }

  return 0;
}


void
benchmark_minver_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_minver();
		
		//portENTER_CRITICAL();
		printf("Initialised minver. Warming up %d\n", 1/*WARMUP_HEAT*/);
		//portEXIT_CRITICAL();
		
		warm_caches_minver(1);
		
		//portENTER_CRITICAL();
		printf("Warmed up minver\n");
		//portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) 0; 
		start_insn = (volatile uint64_t) 0; 
		volatile int result = benchmark_minver ();
		volatile uint64_t end_time = 0;
		volatile uint64_t end_insn = 0;
		
		//portENTER_CRITICAL();
		printf ("minver cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_minver(result));
		if (i==0) {runs_count+=1;}	if (runs_count==0) return;//{//vTaskEndScheduler();}
		//portEXIT_CRITICAL();	
	}

	//vTaskDelete( NULL );
	//vTaskEndScheduler(); // just an infinite loop for RISC-V on FreeRTOS
	return;
}

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
