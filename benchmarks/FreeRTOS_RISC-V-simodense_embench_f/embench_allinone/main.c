/* Common main.c for the benchmarks

   Copyright (C) 2014 Embecosm Limited and University of Bristol
   Copyright (C) 2018-2019 Embecosm Limited

   Contributor: James Pallister <james.pallister@bristol.ac.uk>
   Contributor: Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#include "support.h"


int __attribute__ ((used))
main (int argc __attribute__ ((unused)),
      char *argv[] __attribute__ ((unused)))
{
  int i;
  volatile int result;
  int correct;

/*printf ("Hello!\n");  
printf ("Hello2!\n"); 
long long unsigned int z = 44345461156125;
printf("llu %llu\n",z);*/

//long long unsigned int x = 6467464;
//long long unsigned int u = 7;//64664;
/*u=u<<35;
u=u>>35;
printf ("%d\n",(int)(u));*/
//printf ("%d %d %llu %d\n",sizeof(u),sizeof(u),u,sizeof(u));
//long long unsigned int y = 543454611;
//long long int z = 443454611125;
//printf ("%d\n",(int)((z<<4)>>28));
//printf ("%d\n",(int)((x*u)*(y)/z));
/*long long unsigned int z = 44345461156125;
long long unsigned int y = 54345461651;
printf ("%d\n",(int)((z+1)/(y)));*/
/*/printf ("%llu\n",646576L);
printf ("%llu\n",46545711L);  
printf ("%lld\n",x);  */
//printf ("%llx %llx %llx %llx\n",x, x, x, x);  
//printf ("%u\n", (int) (x/2+x*3+(int)2));  
//printf("%d\n",_WANT_IO_LONG_LONG);
/*int x =1;
for (long long unsigned  i=1; i<64; i+=1){
	//printf("%llu %d\n",(long long unsigned )i,3);
	printf("%d 868249/%llu = %llu   868249 mod %llu = %llu ",0 ,i, (868249L)/ i,(long long unsigned int )i, (long long unsigned int )(( 868249L)% i));
	//printf("868249/%d = %d   868249 mod %d = %d \n",(int)i,(int) (868249/ i),( int )i, (int )(( 868249L)% i));
	printf("%d 868249 * %llu = %llu \n",0,i, (long long unsigned int )( 868249L)* i);
}*/


/*printf ("%llu = %llu*%llu;\n",((long long unsigned int ) 6547232) 
* ((long long unsigned int ) 8932765), (long long unsigned int ) 6547232, (long long unsigned int ) 8932765);
printf ("%llu = %llu*%llu;\n",((long long unsigned int ) 6547232) 
* ((long long unsigned int ) 8932765), (long long unsigned int ) 6547232, (long long unsigned int ) 8932765);
printf ("%lld = %lld*%lld;\n",((long long int ) 6547232) 
* ((long long int ) 8932765), (long long int ) 6547232, (long long  int ) 8932765);
printf ("%llu*%llu;\n", (long long unsigned int ) 6547232, (long long unsigned int ) 8932765);
printf ("%u = %u*%u;\n",((unsigned int ) 6547232) 
* ((unsigned int ) 8932765), (unsigned int ) 6547232, (unsigned int ) 8932765);
printf ("%u = %u/%u;\n",((unsigned int ) 6547232) 
/ ((unsigned int ) 89), (unsigned int ) 6547232, (unsigned int ) 89);
printf ("%u = %u/%u;\n",((unsigned int ) 9876232) 
/ ((unsigned int ) 13), (unsigned int ) 9876232, (unsigned int ) 13);
*/

/*
long long unsigned int x = 131856299UL;
long long unsigned int y = 3107128691UL;
//long long unsigned int z = 2828046579UL;
long long unsigned int t = x*y;//+z;
printf ("t=%d %llx %llx\n",409694489711974609L==t, t, x);
*/
//return 0;

  initialise_board ();
  initialise_benchmark ();
  
  printf("Initialised. Warming up %d\n", WARMUP_HEAT);
  warm_caches (WARMUP_HEAT);
  printf("Warmed up\n");

  start_trigger ();
  result = benchmark ();
  stop_trigger ();

  /* bmarks that use arrays will check a global array rather than int result */
  correct = verify_benchmark (result);
  
  //if (correct) 
  printf("[OK?%d]\n", correct);// Philippos
  
  return (!correct);

}				/* main () */


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
