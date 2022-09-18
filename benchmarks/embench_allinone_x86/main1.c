/* Common main.c for the benchmarks

   Copyright (C) 2014 Embecosm Limited and University of Bristol
   Copyright (C) 2018-2019 Embecosm Limited

   Contributor: James Pallister <james.pallister@bristol.ac.uk>
   Contributor: Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <sys/signal.h>
#include <assert.h>

#include <pthread.h>

#define CPU_MHZ 100
int runs_count=-1;

#include "beebsc.h"


#include "mont64.c"
#include "crc_32.c"
#include "basicmath_small.c"
#include "libedn.c"
#include "libhuffbench.c"
#include "matmult-int.c"
#include "md5.c"
#include "libminver.c"
#include "nbody.c"
#include "nettle-aes.c"
#include "nettle-sha256.c"
#include "libnsichneu.c"
#include "picojpeg_test.c"
#include "primecount.c"
#include "qrtest.c"
#include "combined.c"
#include "libslre.c"
#include "libst.c"
#include "libstatemate.c"
#include "tarfind.c"
#include "libud.c"
#include "libwikisort.c"


int main ()
{
	printf("Main\n");fflush(stdout);

	//pthread_t t[22];
	benchmark_mont64_t(NULL); runs_count=-1;
	benchmark_crc32_t(NULL); runs_count=-1;
	benchmark_cubic_t(NULL); runs_count=-1;
	benchmark_edn_t(NULL); runs_count=-1;
	benchmark_huffbench_t(NULL); runs_count=-1;
	benchmark_matmult_int_t(NULL); runs_count=-1;
	benchmark_md5_t(NULL); runs_count=-1;
	benchmark_minver_t(NULL); runs_count=-1;
	benchmark_nbody_t(NULL); runs_count=-1;
	benchmark_nettle_aes_t(NULL); runs_count=-1;
	benchmark_nettle_sha256_t(NULL); runs_count=-1;
	benchmark_nsichneu_t(NULL); runs_count=-1;
	benchmark_picojpeg_t(NULL); runs_count=-1;
	benchmark_primecount_t(NULL); runs_count=-1;
	benchmark_qrduino_t(NULL); runs_count=-1;
	benchmark_sglib_combined_t(NULL); runs_count=-1;
	benchmark_slre_t(NULL); runs_count=-1;
	benchmark_st_t(NULL); runs_count=-1; 
	benchmark_statemate_t(NULL); runs_count=-1;
	benchmark_tarfind_t(NULL); runs_count=-1;
	benchmark_ud_t(NULL); runs_count=-1;
	benchmark_wikisort_t(NULL); runs_count=-1;
	
	//benchmark_mont64_t(NULL);
	/*pthread_create( &t[0], NULL, benchmark_mont64_t, NULL);
	pthread_create( &t[1], NULL, benchmark_crc32_t, NULL);
	pthread_create( &t[2], NULL, benchmark_cubic_t, NULL);
	pthread_create( &t[3], NULL, benchmark_edn_t, NULL);
	pthread_create( &t[4], NULL, benchmark_huffbench_t, NULL);
	pthread_create( &t[5], NULL, benchmark_matmult_int_t, NULL);
	pthread_create( &t[6], NULL, benchmark_md5_t, NULL);
	pthread_create( &t[7], NULL, benchmark_minver_t, NULL);
	pthread_create( &t[8], NULL, benchmark_nbody_t, NULL);
	pthread_create( &t[9], NULL, benchmark_nettle_aes_t, NULL);
	pthread_create( &t[10], NULL, benchmark_nettle_sha256_t, NULL);
	pthread_create( &t[11], NULL, benchmark_nsichneu_t, NULL);
	pthread_create( &t[12], NULL, benchmark_picojpeg_t, NULL);
	pthread_create( &t[13], NULL, benchmark_primecount_t, NULL);
	pthread_create( &t[14], NULL, benchmark_qrduino_t, NULL);
	pthread_create( &t[15], NULL, benchmark_sglib_combined_t, NULL);
	pthread_create( &t[16], NULL, benchmark_slre_t, NULL);
	pthread_create( &t[17], NULL, benchmark_st_t, NULL);
	pthread_create( &t[18], NULL, benchmark_statemate_t, NULL);
	pthread_create( &t[19], NULL, benchmark_tarfind_t, NULL);
	pthread_create( &t[20], NULL, benchmark_ud_t, NULL);
	pthread_create( &t[21], NULL, benchmark_wikisort_t, NULL);

	//benchmark_statemate_t(NULL);
	for (int i=0; i<22; i++)
		pthread_join( t[i], NULL);*/
    return 0;
}		
