/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* FreeRTOS kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <stdio.h>

#include "riscv-virt.h"

#include "boardsupport.h"

#include "embench_allinone/beebsc.h"
#include "embench_allinone/defs.h"

int runs_count = 0;

#include "embench_allinone/mont64.c"
#include "embench_allinone/crc_32.c"
#include "embench_allinone/basicmath_small.c"
#include "embench_allinone/libedn.c"
#include "embench_allinone/libhuffbench.c"
#include "embench_allinone/matmult-int.c"
#include "embench_allinone/md5.c"
#include "embench_allinone/libminver.c"
#include "embench_allinone/nbody.c"
#include "embench_allinone/nettle-aes.c"
#include "embench_allinone/nettle-sha256.c"
#include "embench_allinone/libnsichneu.c"
#include "embench_allinone/picojpeg_test.c"
#include "embench_allinone/primecount.c"
#include "embench_allinone/qrtest.c"
#include "embench_allinone/combined.c"
#include "embench_allinone/libslre.c"
#include "embench_allinone/libst.c"
#include "embench_allinone/libstatemate.c"
#include "embench_allinone/tarfind.c"
#include "embench_allinone/libud.c"
#include "embench_allinone/libwikisort.c"

/* Priorities used by the tasks. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 5 ) //000 )

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
#define mainQUEUE_LENGTH					( 5 )

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
	TickType_t xNextWakeTime;
	unsigned long ulValueToSend = 0UL;

	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	
	portENTER_CRITICAL();
	printf("TaskA\n");
	portEXIT_CRITICAL();
	
	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again. */
		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		ulValueToSend++;

		char buf[40];
		sprintf( buf, "%d: %s: send %ld\n", xGetCoreID(),
				pcTaskGetName( xTaskGetCurrentTaskHandle() ),
				ulValueToSend );
		/*vSendString*/
		portENTER_CRITICAL();
		printf( buf );
		portEXIT_CRITICAL();
		/* 0 is used as the block time so the sending operation will not block -
		 * it shouldn't need to block as the queue should always be empty at
		 * this point in the code. */
		xQueueSend( xQueue, &ulValueToSend, 0U );
		//printf("sending\n");
	}
}

/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	portENTER_CRITICAL();
	printf("TaskB\n");
	portEXIT_CRITICAL();
		
	for( ;; )
	{

		unsigned long ulReceivedValue;
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
		//printf("TaskB\n");
		/*  To get here something must have been received from the queue. */
		char buf[40];
		sprintf( buf, "%d: %s: received %ld\n", xGetCoreID(),
				pcTaskGetName( xTaskGetCurrentTaskHandle() ),
				ulReceivedValue );
		/*vSendString*/
		portENTER_CRITICAL();
		printf( buf );
		portEXIT_CRITICAL();
		//printf("receiving\n");
	}
}

/*-----------------------------------------------------------*/



static void spin( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	portENTER_CRITICAL();
	//printf("ok\n");
	portEXIT_CRITICAL();
	
	for( int i=0; i<5; i++ )
	{
		printf("1\n");		
	}
	
	//vTaskDelete( NULL );
	printf("here\n");
	//vTaskSuspendAll();
	//vTaskSuspend(NULL);
	vTaskEndScheduler(); // just an infinite loop for RISC-V
	return;
}

static void spin2( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	portENTER_CRITICAL();
	//printf("ok\n");
	portEXIT_CRITICAL();
	
	for( int i=0; i<50; i++ )
	{
		printf("2\n");		
	}
	//vTaskDelete( NULL );
	vTaskEndScheduler();
}



int main_blinky( void )
{
	//initialise_board();
	/*vSendString*/
	printf( "Hello FreeRTOS!\n" );
	
	//unsigned long long int z = insn();//+1;
	//unsigned long long int z2 = time();//+2;
	
	/*uint32_t cycles_low;
	asm volatile ("rdcycle %0" : "=r"(cycles_low));
	uint32_t cycles_high;	
	asm volatile ("rdcycleh %0" : "=r"(cycles_high));
	unsigned long long int z3 = ((uint64_t)cycles_high<<32)|cycles_low;
	printf("%llx\n", z3);
	printf("%llu\n", z3);
	
	printf("%d %llu %llu %d\n", 0, z, time()+2,0);
	printf("%d %llu %llu %d\n", 0, z, z3,0);
	return;*/
	
	/*unsigned long long int z = insn();
	unsigned long long int z2 = insn();
	printf("%d %llu %llu %llu %d\n", 0, z, z2, z2-z,0);
	return;*/
	
	/* Create the queue. */
	printf("Creating queue...\n");
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( unsigned long ) );
	printf("OK\n");
		
	if( xQueue != NULL )
	{
#ifdef Be0
            printf("Create task aha-mont64\n");
            xTaskCreate(benchmark_mont64_t, "aha-mont64", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be1
            printf("Create task crc32\n");
            xTaskCreate(benchmark_crc32_t, "crc32", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be2
            printf("Create task cubic\n");
            xTaskCreate(benchmark_cubic_t, "cubic", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be3
            printf("Create task edn\n");
            xTaskCreate(benchmark_edn_t, "edn", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be4
            printf("Create task huffbench\n");
            xTaskCreate(benchmark_huffbench_t, "huffbench", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be5
            printf("Create task matmult-int\n");
            xTaskCreate(benchmark_matmult_int_t, "matmult-int", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be6
            printf("Create task md5sum\n");
            xTaskCreate(benchmark_md5sum_t, "md5sum", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be7
            printf("Create task minver\n");
            xTaskCreate(benchmark_minver_t, "minver", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be8
            printf("Create task nbody\n");
            xTaskCreate(benchmark_nbody_t, "nbody", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be9
            printf("Create task nettle-aes\n");
            xTaskCreate(benchmark_nettle_aes_t, "nettle-aes", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be10
            printf("Create task nettle-sha256\n");
            xTaskCreate(benchmark_nettle_sha256_t, "nettle-sha256", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be11
            printf("Create task nsichneu\n");
            xTaskCreate(benchmark_nsichneu_t, "nsichneu", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be12
            printf("Create task picojpeg\n");
            xTaskCreate(benchmark_picojpeg_t, "picojpeg", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be13
            printf("Create task primecount\n");
            xTaskCreate(benchmark_primecount_t, "primecount", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be14
            printf("Create task qrduino\n");
            xTaskCreate(benchmark_qrduino_t, "qrduino", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be15
            printf("Create task sglib-combined\n");
            xTaskCreate(benchmark_sglib_combined_t, "sglib-combined", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be16
            printf("Create task slre\n");
            xTaskCreate(benchmark_slre_t, "slre", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be17
            printf("Create task st\n");
            xTaskCreate(benchmark_st_t, "st", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be18
            printf("Create task statemate\n");
            xTaskCreate(benchmark_statemate_t, "statemate", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be19
            printf("Create task tarfind\n");
            xTaskCreate(benchmark_tarfind_t, "tarfind", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be20
            printf("Create task ud\n");
            xTaskCreate(benchmark_ud_t, "ud", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif
#ifdef Be21
            printf("Create task wikisort\n");
            xTaskCreate(benchmark_wikisort_t, "wikisort", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
#endif

	}
	
	printf("Starting scheduler...\n");
	vTaskStartScheduler();
	printf("After scheduler.\n");
	
	//printf("Exiting FreeRTOS! Good bye\n");
	return 0;
}
