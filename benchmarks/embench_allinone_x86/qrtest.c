/* BEEBS qrduino benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Original code from: https://github.com/tz1/qrduino */

//#include "boardsupport.h"
//#include "qrencode.h"
#include "qrencode.c"

#include <string.h>

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR_qrduino 5

/* BEEBS heap is just an array */

#define HEAP_SIZE_qrduino 8192
static char heap_qrduino[HEAP_SIZE_qrduino];

static void *heap_ptr_qrduino = NULL;
static void *heap_end_qrduino = NULL;
static size_t heap_requested_qrduino = 0;

#include "qrframe.c"

static const char *encode;
static int size_qrduino;

static int benchmark_body_qrduino (int  rpt);

static void
warm_caches_qrduino (int  heat)
{
  int  res = benchmark_body_qrduino (heat);

  return;
}


static int
benchmark_qrduino (void)
{
  return benchmark_body_qrduino (LOCAL_SCALE_FACTOR_qrduino * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body_qrduino (int rpt)
{
  static const char *in_encode = "http://www.mageec.com";
  int i;

  for (i = 0; i < rpt; i++)
    {
      encode = in_encode;
      size_qrduino = 22;
      init_heap_beebs (&heap_ptr_qrduino, &heap_end_qrduino, &heap_requested_qrduino,
      	(void *) heap_qrduino, HEAP_SIZE_qrduino);

      initeccsize (1, size_qrduino);

      memcpy (strinbuf, encode, size_qrduino);

      initframe ();
      qrencode ();
      freeframe ();
      freeecc ();
    }

  return 0;
}

static void
initialise_benchmark_qrduino ()
{
}

static int
verify_benchmark_qrduino (int unused)
{
  unsigned char expected_qrduino[22] = {
    254, 101, 63, 128, 130, 110, 160, 128, 186, 65, 46,
    128, 186, 38, 46, 128, 186, 9, 174, 128, 130, 20
  };

  return (0 == memcmp (strinbuf, expected_qrduino, 22 * sizeof (strinbuf[0])))
    && check_heap_beebs (&heap_ptr_qrduino, &heap_end_qrduino, &heap_requested_qrduino,
    			(void *) heap_qrduino);
}

void
benchmark_qrduino_t (void* argv)
{
	uint64_t start_time = 0;
	uint64_t start_insn = 0;
	
	for (int i=0; i<200; i++ ){
		initialise_benchmark_qrduino();
		
		//portENTER_CRITICAL();
		printf("Initialised qrduino. Warming up %d\n", 1/*WARMUP_HEAT*/);
		//portEXIT_CRITICAL();
		
		warm_caches_qrduino(1);
		
		//portENTER_CRITICAL();
		printf("Warmed up qrduino\n");
		//portEXIT_CRITICAL();
		
		start_time = (volatile uint64_t) 0; 
		start_insn = (volatile uint64_t) 0; 
		volatile int result = benchmark_qrduino ();
		volatile uint64_t end_time = 0;
		volatile uint64_t end_insn = 0;
		
		//portENTER_CRITICAL();
		printf ("qrduino cycles %llu instructions %llu ", end_time-start_time, end_insn-start_insn);
		printf("[OK?%d]\n", verify_benchmark_qrduino(result));
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
