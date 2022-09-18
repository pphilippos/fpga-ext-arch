/* BEEBS st benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/* stats.c */

/* 2012/09/28, Jan Gustafsson <jan.gustafsson@mdh.se>
 * Changes:
 *  - time is only enabled if the POUT flag is set
 *  - st.c:30:1:  main () warning: type specifier missing, defaults to 'int':
 *    fixed
 */


/* 2011/10/18, Benedikt Huber <benedikt@vmars.tuwien.ac.at>
 * Changes:
 *  - Measurement and Printing the Results is only enabled if the POUT flag is
 *    set
 *  - Added Prototypes for InitSeed and RandomInteger
 *  - Changed return type of InitSeed from 'missing (default int)' to 'void'
 */

#include <math.h>
//#include "boardsupport.h"

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_st 13

#define MAX_st 100

void InitSeed_st (void);
int RandomInteger_st ();
void Initialize_st (float[]);
void Calc_Sum_Mean (float[], float *, float *);
void Calc_Var_Stddev (float[], float, float *, float *);
void Calc_LinCorrCoef (float[], float[], float, float);


/* Statistics Program:
 * This program computes for two arrays of numbers the sum, the
 * mean, the variance, and standard deviation.  It then determines the
 * correlation coefficient between the two arrays.
 */

int Seed_st;
float ArrayA_st[MAX_st], ArrayB_st[MAX_st];
float SumA, SumB;
float Coef;


static void
initialise_benchmark_st (void)
{
}


static int benchmark_body_st (int  rpt);

static void
warm_caches_st (int  heat)
{
  int  res = benchmark_body_st (heat);

  return;
}


static int
benchmark_st (void)
{
  return benchmark_body_st (LOCAL_SCALE_FACTOR_st * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body_st (int rpt)
{
  int i;

  for (i = 0; i < rpt; i++)
    {
      float MeanA, MeanB, VarA, VarB, StddevA, StddevB /*, Coef */ ;

      InitSeed_st ();

      Initialize_st (ArrayA_st);
      Calc_Sum_Mean (ArrayA_st, &SumA, &MeanA);
      Calc_Var_Stddev (ArrayA_st, MeanA, &VarA, &StddevA);

      Initialize_st (ArrayB_st);
      Calc_Sum_Mean (ArrayB_st, &SumB, &MeanB);
      Calc_Var_Stddev (ArrayB_st, MeanB, &VarB, &StddevB);

      /* Coef will have to be used globally in Calc_LinCorrCoef since it would
         be beyond the 6 registers used for passing parameters
       */
      Calc_LinCorrCoef (ArrayA_st, ArrayB_st, MeanA, MeanB /*, &Coef */ );
    }

  return 0;
}


void
InitSeed_st ()
/*
 * Initializes the seed used in the random number generator.
 */
{
  Seed_st = 0;
}


void
Calc_Sum_Mean (float Array[], float *Sum, float *Mean)
{
  int i;

  *Sum = 0;
  for (i = 0; i < MAX_st; i++)
    *Sum += Array[i];
  *Mean = *Sum / MAX_st;
}


float
Square (float x)
{
  return x * x;
}


void
Calc_Var_Stddev (float Array[], float Mean, float *Var, float *Stddev)
{
  int i;
  float diffs;

  diffs = 0.0;
  for (i = 0; i < MAX_st; i++)
    diffs += Square (Array[i] - Mean);
  *Var = diffs / MAX_st;
  *Stddev = sqrtf (*Var);
}


void
Calc_LinCorrCoef (float ArrayA_[], float ArrayB_[], float MeanA,
		  float MeanB /*, Coef */ )
{
  int i;
  float numerator, Aterm, Bterm;

  numerator = 0.0;
  Aterm = Bterm = 0.0;
  for (i = 0; i < MAX_st; i++)
    {
      numerator += (ArrayA_[i] - MeanA) * (ArrayB_[i] - MeanB);
      Aterm += Square (ArrayA_[i] - MeanA);
      Bterm += Square (ArrayB_[i] - MeanB);
    }

  /* Coef used globally */
  Coef = numerator / (sqrtf (Aterm) * sqrtf (Bterm));
}



void
Initialize_st (float Array[])
/*
 * Intializes the given array with random integers.
 */
{
  register int i;

  for (i = 0; i < MAX_st; i++)
    Array[i] = i + RandomInteger_st () / 8095.0;
}


int
RandomInteger_st ()
/*
 * Generates random integers between 0 and 8095
 */
{
  Seed_st = ((Seed_st * 133) + 81) % 8095;
  return (Seed_st);
}

static int
verify_benchmark_st (int unused)
{
  float expSumA = 4999.002930;//4999.00247066090196;
  float expSumB = 4996.843750;//4996.84311303273534;
  float expCoef = 0.999900;//0.999900054853619324;

  return float_eq_beebs(expSumA, SumA)
    && float_eq_beebs(expSumB, SumB)
    && float_eq_beebs(expCoef, Coef);
}

void
benchmark_st_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_st();
		
		//portENTER_CRITICAL();
		printf("Initialised st. Warming up %d\n", 1/*WARMUP_HEAT*/);
		//portEXIT_CRITICAL();
		
		warm_caches_st(1);
		
		//portENTER_CRITICAL();
		printf("Warmed up st\n");
		//portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) 0; 
		start_insn = (volatile uint64_t) 0; 
		volatile int result = benchmark_st ();
		volatile uint64_t end_time = 0;
		volatile uint64_t end_insn = 0;
		
		//portENTER_CRITICAL();
		printf ("st cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_st(result));
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
