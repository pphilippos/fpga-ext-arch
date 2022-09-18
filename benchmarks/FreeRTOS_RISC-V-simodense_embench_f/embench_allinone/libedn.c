/* BEEBS edn benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Original code from: WCET Benchmarks,
       http://www.mrtc.mdh.se/projects/wcet/benchmarks.html

   Permission to license under GPL obtained by email from Björn Lisper */

/*
 * MDH WCET BENCHMARK SUITE.
 */

/************************************************************************
*	Simple vector multiply				*
************************************************************************/

/*
 * Changes: JG 2005/12/22: Inserted prototypes, changed type of main to int
 * etc. Added parenthesis in expressions in jpegdct. Removed unused variable
 * dx. Changed int to long to avoid problems when compiling to 16 bit target
 * Indented program.
 * JG 2006-01-27: Removed code in codebook
 */

#include <string.h>
#include "boardsupport.h"

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_edn 87


#define N_edn 100
#define ORDER 50

void vec_mpy1 (short y[], const short x[], short scaler);
long int mac (const short *a, const short *b, long int sqr, long int *sum);
void fir (const short array1[], const short coeff[], long int output[]);
void fir_no_red_ld (const short x[], const short h[], long int y[]);
long int latsynth (short b[], const short k[], long int n, long int f);
void iir1 (const short *coefs, const short *input, long int *optr,
	   long int *state);
long int codebook (long int mask, long int bitchanged, long int numbasis,
		   long int codeword, long int g, const short *d, short ddim,
		   short theta);
void jpegdct (short *d, short *r);

void
vec_mpy1 (short y[], const short x[], short scaler)
{
  long int i;

  for (i = 0; i < 150; i++)
    y[i] += ((scaler * x[i]) >> 15);
}


/*****************************************************
*			Dot Product	      *
*****************************************************/
long int
mac (const short *a, const short *b, long int sqr, long int *sum)
{
  long int i;
  long int dotp = *sum;

  for (i = 0; i < 150; i++)
    {
      dotp += b[i] * a[i];
      sqr += b[i] * b[i];
    }

  *sum = dotp;
  return sqr;
}


/*****************************************************
*		FIR Filter		     *
*****************************************************/
void
fir (const short array1[], const short coeff[], long int output[])
{
  long int i, j, sum;

  for (i = 0; i < N_edn - ORDER; i++)
    {
      sum = 0;
      for (j = 0; j < ORDER; j++)
	{
	  sum += array1[i + j] * coeff[j];
	}
      output[i] = sum >> 15;
    }
}

/****************************************************
*	FIR Filter with Redundant Load Elimination

By doing two outer loops simultaneously, you can potentially  reuse data (depending on the DSP architecture).
x and h  only  need to be loaded once, therefore reducing redundant loads.
This reduces memory bandwidth and power.
*****************************************************/
void
fir_no_red_ld (const short x[], const short h[], long int y[])
{
  long int i, j;
  long int sum0, sum1;
  short x0, x1, h0, h1;
  for (j = 0; j < 100; j += 2)
    {
      sum0 = 0;
      sum1 = 0;
      x0 = x[j];
      for (i = 0; i < 32; i += 2)
	{
	  x1 = x[j + i + 1];
	  h0 = h[i];
	  sum0 += x0 * h0;
	  sum1 += x1 * h0;
	  x0 = x[j + i + 2];
	  h1 = h[i + 1];
	  sum0 += x1 * h1;
	  sum1 += x0 * h1;
	}
      y[j] = sum0 >> 15;
      y[j + 1] = sum1 >> 15;
    }
}

/*******************************************************
*	Lattice Synthesis	           *
* This function doesn't follow the typical DSP multiply two  vector operation, but it will point out the compiler's flexibility   ********************************************************/
long int
latsynth (short b[], const short k[], long int n, long int f)
{
  long int i;

  f -= b[n - 1] * k[n - 1];
  for (i = n - 2; i >= 0; i--)
    {
      f -= b[i] * k[i];
      b[i + 1] = b[i] + ((k[i] * (f >> 16)) >> 16);
    }
  b[0] = f >> 16;
  return f;
}

/*****************************************************
*			IIR Filter		     *
*****************************************************/
void
iir1 (const short *coefs, const short *input, long int *optr, long int *state)
{
  long int x;
  long int t;
  long int n;

  x = input[0];
  for (n = 0; n < 50; n++)
    {
      t = x + ((coefs[2] * state[0] + coefs[3] * state[1]) >> 15);
      x = t + ((coefs[0] * state[0] + coefs[1] * state[1]) >> 15);
      state[1] = state[0];
      state[0] = t;
      coefs += 4;		/* point to next filter coefs  */
      state += 2;		/* point to next filter states */
    }
  *optr++ = x;
}

/*****************************************************
*	Vocoder Codebook Search 	     *
*****************************************************/
long int
codebook (long int mask, long int bitchanged, long int numbasis,
	  long int codeword, long int g, const short *d, short ddim,
	  short theta)
/*
 * dfm (mask=d  bitchanged=1 numbasis=17  codeword=e[0] , g=d, d=a, ddim=c,
 * theta =1
 */
{
  long int j;


  /*
   * Remove along with the code below.
   *
   long int        tmpMask;

   tmpMask = mask << 1;
   */
  for (j = bitchanged + 1; j <= numbasis; j++)
    {



/*
 * The following code is removed since it gave a memory access exception.
 * It is OK since the return value does not control the flow.
 * The loop always iterates a fixed number of times independent of the loop body.

    if (theta == !(!(codeword & tmpMask)))
			g += *(d + bitchanged * ddim + j);
		else
			g -= *(d + bitchanged * ddim + j);
		tmpMask <<= 1;
*/
    }
  return g;
}


/*****************************************************
*		JPEG Discrete Cosine Transform 		     *
*****************************************************/
void
jpegdct (short *d, short *r)
{
  long int t[12];
  short i, j, k, m, n, p;
  for (k = 1, m = 0, n = 13, p = 8; k <= 8;
       k += 7, m += 3, n += 3, p -= 7, d -= 64)
    {
      for (i = 0; i < 8; i++, d += p)
	{
	  for (j = 0; j < 4; j++)
	    {
	      t[j] = d[k * j] + d[k * (7 - j)];
	      t[7 - j] = d[k * j] - d[k * (7 - j)];
	    }
	  t[8] = t[0] + t[3];
	  t[9] = t[0] - t[3];
	  t[10] = t[1] + t[2];
	  t[11] = t[1] - t[2];
	  d[0] = (t[8] + t[10]) >> m;
	  d[4 * k] = (t[8] - t[10]) >> m;
	  t[8] = (short) (t[11] + t[9]) * r[10];
	  d[2 * k] = t[8] + (short) ((t[9] * r[9]) >> n);
	  d[6 * k] = t[8] + (short) ((t[11] * r[11]) >> n);
	  t[0] = (short) (t[4] + t[7]) * r[2];
	  t[1] = (short) (t[5] + t[6]) * r[0];
	  t[2] = t[4] + t[6];
	  t[3] = t[5] + t[7];
	  t[8] = (short) (t[2] + t[3]) * r[8];
	  t[2] = (short) t[2] * r[1] + t[8];
	  t[3] = (short) t[3] * r[3] + t[8];
	  d[7 * k] = (short) (t[4] * r[4] + t[0] + t[2]) >> n;
	  d[5 * k] = (short) (t[5] * r[6] + t[1] + t[3]) >> n;
	  d[3 * k] = (short) (t[6] * r[5] + t[1] + t[2]) >> n;
	  d[1 * k] = (short) (t[7] * r[7] + t[0] + t[3]) >> n;
	}
    }
}

static short a_edn[200];
static short b_edn[200];
static short c_edn;
static long int d_edn;
static int e_edn;
static long int output[200];


static void
initialise_benchmark_edn (void)
{
}


static int benchmark_body_edn (int  rpt);

static void
warm_caches_edn (int  heat)
{
  int  res = benchmark_body_edn (heat);

  return;
}


static int
benchmark_edn (void)
{
  return benchmark_body_edn (LOCAL_SCALE_FACTOR_edn * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body_edn (int rpt)
{
  int j;

  for (j = 0; j < rpt; j++)
    {
      short unsigned int in_a[200] = {
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400,
	0x0000, 0x07ff, 0x0c00, 0x0800, 0x0200, 0xf800, 0xf300, 0x0400
      };
      short unsigned int in_b[200] = {
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000,
	0x0c60, 0x0c40, 0x0c20, 0x0c00, 0xf600, 0xf400, 0xf200, 0xf000
      };
      c_edn = 0x3;
      d_edn = 0xAAAA;
      e_edn = 0xEEEE;

      for (int i = 0; i < 200; i++)
	{
	  a_edn[i] = in_a[i];
	  b_edn[i] = in_b[i];
	}
      /*
       * Declared as memory variable so it doesn't get optimized out
       */

      vec_mpy1 (a_edn, b_edn, c_edn);
      c_edn = mac (a_edn, b_edn, (long int) c_edn, (long int *) output);
      fir (a_edn, b_edn, output);
      fir_no_red_ld (a_edn, b_edn, output);
      d_edn = latsynth (a_edn, b_edn, N_edn, d_edn);
      iir1 (a_edn, b_edn, &output[100], output);
      e_edn = codebook (d_edn, 1, 17, e_edn, d_edn, a_edn, c_edn, 1);
      jpegdct (a_edn, b_edn);
    }
  return 0;
}

static int
verify_benchmark_edn (int unused)
{
  long int exp_output[200] =
    { 3760, 4269, 3126, 1030, 2453, -4601, 1981, -1056, 2621, 4269,
    3058, 1030, 2378, -4601, 1902, -1056, 2548, 4269, 2988, 1030,
    2300, -4601, 1822, -1056, 2474, 4269, 2917, 1030, 2220, -4601,
    1738, -1056, 2398, 4269, 2844, 1030, 2140, -4601, 1655, -1056,
    2321, 4269, 2770, 1030, 2058, -4601, 1569, -1056, 2242, 4269,
    2152, 1030, 1683, -4601, 1627, -1056, 2030, 4269, 2080, 1030,
    1611, -4601, 1555, -1056, 1958, 4269, 2008, 1030, 1539, -4601,
    1483, -1056, 1886, 4269, 1935, 1030, 1466, -4601, 1410, -1056,
    1813, 4269, 1862, 1030, 1393, -4601, 1337, -1056, 1740, 4269,
    1789, 1030, 1320, -4601, 1264, -1056, 1667, 4269, 1716, 1030,
    1968, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  return (0 == memcmp (output, exp_output, 200 * sizeof (output[0])))
    && (10243 == c_edn) && (-441886230 == d_edn) && (-441886230 == e_edn);
}


void
benchmark_edn_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_edn();
		
		portENTER_CRITICAL();
		printf("Initialised edn. Warming up %d\n", 1/*WARMUP_HEAT*/);
		portEXIT_CRITICAL();
		
		warm_caches_edn(1);
		
		portENTER_CRITICAL();
		printf("Warmed up edn\n");
		portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) time(); 
		start_insn = (volatile uint64_t) insn(); 
		volatile int result = benchmark_edn ();
		volatile uint64_t end_time = time();
		volatile uint64_t end_insn = insn();
		
		portENTER_CRITICAL();
		printf ("edn cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_edn(result));
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
