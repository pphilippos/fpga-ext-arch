// Author: Philippos Papaphilippou 2022
//
// Note/Acknowledgement:
// - Based on pinatrace.cpp from Intel
// - The Hashset implementation is from https://github.com/avsej/hashset.c)

/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2016 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */
 
#include <iostream>
#include <fstream>

#include <stdio.h>
#include "pin.H"

#include <map>

//#include "hashset.c/hashset.h"

// Hashset implementation is from https://github.com/avsej/hashset.c

#ifndef HASHSET_H
#define HASHSET_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct hashset_st {
        size_t nbits;
        size_t mask;

        size_t capacity;
        size_t *items;
        size_t nitems;
        size_t n_deleted_items;
    };

    typedef struct hashset_st *hashset_t;

    /* create hashset instance */
    hashset_t hashset_create(void);

    /* destroy hashset instance */
    void hashset_destroy(hashset_t set);

    size_t hashset_num_items(hashset_t set);

    /* add item into the hashset.
     *
     * @note 0 and 1 is special values, meaning nil and deleted items. the
     *       function will return -1 indicating error.
     *
     * returns zero if the item already in the set and non-zero otherwise
     */
    int hashset_add(hashset_t set, void *item);

    /* remove item from the hashset
     *
     * returns non-zero if the item was removed and zero if the item wasn't
     * exist
     */
    int hashset_remove(hashset_t set, void *item);

    /* check if existence of the item
     *
     * returns non-zero if the item exists and zero otherwise
     */
    int hashset_is_member(hashset_t set, void *item);

#ifdef __cplusplus
}
#endif

#endif


#ifndef HASHSET_H
#define HASHSET_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct hashset_st {
        size_t nbits;
        size_t mask;

        size_t capacity;
        size_t *items;
        size_t nitems;
        size_t n_deleted_items;
    };

    typedef struct hashset_st *hashset_t;

    /* create hashset instance */
    hashset_t hashset_create(void);

    /* destroy hashset instance */
    void hashset_destroy(hashset_t set);

    size_t hashset_num_items(hashset_t set);

    /* add item into the hashset.
     *
     * @note 0 and 1 is special values, meaning nil and deleted items. the
     *       function will return -1 indicating error.
     *
     * returns zero if the item already in the set and non-zero otherwise
     */
    int hashset_add(hashset_t set, void *item);

    /* remove item from the hashset
     *
     * returns non-zero if the item was removed and zero if the item wasn't
     * exist
     */
    int hashset_remove(hashset_t set, void *item);

    /* check if existence of the item
     *
     * returns non-zero if the item exists and zero otherwise
     */
    int hashset_is_member(hashset_t set, void *item);

#ifdef __cplusplus
}
#endif

#endif



#include <semaphore.h>
sem_t m;


//#include <tr1/unordered_set>

//#include <unordered_set>
//#include <boost/unordered_map.hpp>
//typedef boost::unordered_map<std::string, int> unordered_set;


FILE * trace; 

KNOB<UINT64> KnobIFWD(KNOB_MODE_WRITEONCE, "pintool", "ifwd", "0", "Millions instructions to be forwarded");
KNOB<UINT64> KnobILIMIT(KNOB_MODE_WRITEONCE, "pintool", "ilimit", "0", "representative region icount, in Millions");
KNOB<UINT32> KnobThread(KNOB_MODE_WRITEONCE, "pintool", "thread", "0", "Thread to observe");
KNOB<std::string> KnobString(KNOB_MODE_WRITEONCE, "pintool", "out", "pinatrace2e.out0", "Thread to observe");
//KNOB<UINT64> KnobSS(KNOB_MODE_WRITEONCE, "pintool", "bucket_size", "128", "set size for unique units (opcodes, instruction/memory blocks)");

static UINT32 observed_thread = 0;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static UINT64 icount_limit = 0;
static UINT64 icount_fwd = 0;
//static UINT64 bucket_size = 1024;

int bucket_sizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};
#define nbuckets 15

long long sample_counts[nbuckets] = {0};

std::map<unsigned int, UINT64> unique_counts_op [nbuckets];
std::map<unsigned int, UINT64> unique_counts_ip [nbuckets];
std::map<unsigned int, UINT64> unique_counts_ad [nbuckets];

//std::/*unordered_*/set <UINT32> sets_op[nbuckets];
//std::/*unordered_*/set <UINT64> sets_ip[nbuckets];
//std::/*unordered_*/set <UINT64> sets_ad[nbuckets];
hashset_t sets_op[nbuckets];
hashset_t sets_ip[nbuckets];
hashset_t sets_ad[nbuckets];

int block_offset_bits = 6; // 64 bytes

VOID Fini(INT32 code, VOID *v);

PIN_MUTEX Mutex;

// This function is called before every instruction is executed
VOID docount() { 
	//if (PIN_ThreadId() != observed_thread) return;
	PIN_MutexLock(&Mutex);
	
	icount++;
	if (icount == icount_limit){ // (if icount_limit is 0, there isn't any limit (default value))
		Fini(0,0);
		std::cout << "icount " << icount << std::endl;
        fclose(trace);
        exit(0);
	} 
	for (int i=0; i<nbuckets; i++){	
		if (icount%bucket_sizes[i]==0) {
			sample_counts[i]++;
			
			unsigned int sop = hashset_num_items(sets_op[i]);  
			unsigned int sip = hashset_num_items(sets_ip[i]); 
			unsigned int sad = hashset_num_items(sets_ad[i]); 
			
			hashset_destroy(sets_op[i]); sets_op[i] = hashset_create();
			hashset_destroy(sets_ip[i]); sets_ip[i] = hashset_create();
			hashset_destroy(sets_ad[i]); sets_ad[i] = hashset_create();
			
			auto kvpair_it = unique_counts_op[i].find(sop);
			if (kvpair_it == unique_counts_op[i].end())
				unique_counts_op[i].insert(std::pair<unsigned, UINT64>(sop,1));
			else kvpair_it->second++;			
			
			kvpair_it = unique_counts_ip[i].find(sip);	
			if (kvpair_it == unique_counts_ip[i].end()) 
				unique_counts_ip[i].insert(std::pair<unsigned, UINT64>(sip,1)); 
			else kvpair_it->second++;
				
			kvpair_it = unique_counts_ad[i].find(sad);	
			if (kvpair_it == unique_counts_ad[i].end()) 	
				unique_counts_ad[i].insert(std::pair<unsigned, UINT64>(sad,1)); 
			else kvpair_it->second++;			
			
		}
	}
	PIN_MutexUnlock(&Mutex);
}

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr, UINT32 opcode)
{
	PIN_MutexLock(&Mutex);
	//if (PIN_ThreadId() != observed_thread) return;
	
    //fprintf(trace,"%p: R %p\n", ip, addr);
    if (icount>=icount_fwd){
    	//fprintf(trace,"ip %p op 0x%x %s: tid %d icount %llu R 0x%llx\n", ip, opcode, OPCODE_StringShort(opcode).c_str(), PIN_ThreadId(), (unsigned long long) icount, (unsigned long long) addr);
    	for (int i=0; i<nbuckets; i++){	
			hashset_add(sets_op[i],(void*)((UINT64)opcode));//(UINT32)
			hashset_add(sets_ip[i],(void*)(((UINT64)  ip)>>block_offset_bits));
			hashset_add(sets_ad[i],(void*)(((UINT64)addr)>>block_offset_bits));//<<block_offset_bits);
		}
    }
    PIN_MutexUnlock(&Mutex);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 opcode)
{
	//if (PIN_ThreadId() != observed_thread) return;
	PIN_MutexLock(&Mutex);
	
    //fprintf(trace,"%p: W %p\n", ip, addr);
    if (icount>=icount_fwd){
    	//fprintf(trace,"ip %p op 0x%x %s: tid %d icount %llu W 0x%llx\n", ip, opcode, OPCODE_StringShort(opcode).c_str(), PIN_ThreadId(), (unsigned long long) icount, (unsigned long long) addr);
    	for (int i=0; i<nbuckets; i++){	
			hashset_add(sets_op[i],(void*)((UINT64)opcode));//(UINT32)
			hashset_add(sets_ip[i],(void*)(((UINT64)  ip)>>block_offset_bits));
			hashset_add(sets_ad[i],(void*)(((UINT64)addr)>>block_offset_bits));//<<block_offset_bits);
		}
    }
    PIN_MutexUnlock(&Mutex);
}

VOID RecordOther(VOID * ip, UINT32 opcode)
{
	//if (PIN_ThreadId() != observed_thread) return;
	PIN_MutexLock(&Mutex);
	
    //fprintf(trace,"%p: R %p\n", ip, addr);
    if (icount>=icount_fwd){
    	//fprintf(trace,"ip %p op 0x%x %s: tid %d icount %llu\n", ip, opcode, OPCODE_StringShort(opcode).c_str(), PIN_ThreadId(), (unsigned long long) icount);
    	for (int i=0; i<nbuckets; i++){	
			hashset_add(sets_op[i],(void*)((UINT64)opcode));//(UINT32)
			hashset_add(sets_ip[i],(void*)(((UINT64)  ip)>>block_offset_bits));
		}
    }
    PIN_MutexUnlock(&Mutex);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
   // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
    
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);
	UINT32 opcode = INS_Opcode(ins);

	bool was_memoryp=false;
	
    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))        
        {
        	was_memoryp=true;
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, 
                IARG_UINT32, opcode,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {	
        	was_memoryp=true;   
        	INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, 
                IARG_UINT32, opcode,
                IARG_END);         
        }        
    }
    if (!was_memoryp){
    	INS_InsertCall( //INS_InsertPredicatedCall
            ins, IPOINT_BEFORE, (AFUNPTR)RecordOther,
            IARG_INST_PTR,                
            IARG_UINT32, opcode,
            IARG_END);
    }
}

unsigned find_percentile(std::map<unsigned, UINT64> pairs, int perc, long long total){
	UINT64 target = total*perc/100;
	UINT64 sum = 0;
	
	for (auto it = pairs.begin(); it != pairs.end(); ++it){
		sum += it->second;
		if (sum >= target)
			return it->first;
	}
	return 0;
}

VOID Fini(INT32 code, VOID *v)
{	
	for (int i=0; i<nbuckets; i++){			
		
		for (auto it = unique_counts_op[i].begin(); 
			it != unique_counts_op[i].end(); ++it) {
			fprintf(trace, "opcodes window %d key %u frequency %lu\n", 
			bucket_sizes[i], it->first, it->second);
		}		
		for (auto it = unique_counts_ip[i].begin(); 
			it != unique_counts_ip[i].end(); ++it) {
			fprintf(trace, "instr window %d key %u frequency %lu\n", 
			bucket_sizes[i], it->first, it->second);
		}		
		for (auto it = unique_counts_ad[i].begin(); 
			it != unique_counts_ad[i].end(); ++it) {
			fprintf(trace, "data window %d key %u frequency %lu\n", 
			bucket_sizes[i], it->first, it->second);
		}		
		
	}
	//while (true)
	//std::cerr << "icount " << icount << std::endl;
	fprintf(trace, "icount %lu\n", icount); 
    //fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

	icount_limit = KnobILIMIT.Value()*1000000; 
	icount_fwd   = KnobIFWD  .Value()*1000000;
	observed_thread = KnobThread.Value();
	//bucket_size  = KnobSS.Value(); 
	for (int i=0; i<nbuckets; i++){
		sets_op[i]=hashset_create();
		sets_ip[i]=hashset_create();
		sets_ad[i]=hashset_create();
	}

    trace = /*stdout;//*/fopen(KnobString.Value().c_str(), "w");
	
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}
















/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2012 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */





static const unsigned int prime_1 = 73;
static const unsigned int prime_2 = 5009;

hashset_t hashset_create()
{
    hashset_t set = (hashset_t) calloc(1, sizeof(struct hashset_st));

    if (set == NULL) {
        return NULL;
    }
    set->nbits = 3;
    set->capacity = (size_t)(1 << set->nbits);
    set->mask = set->capacity - 1;
    set->items = (size_t *) calloc(set->capacity, sizeof(size_t));
    if (set->items == NULL) {
        hashset_destroy(set);
        return NULL;
    }
    set->nitems = 0;
    set->n_deleted_items = 0;
    return set;
}

size_t hashset_num_items(hashset_t set)
{
    return set->nitems;
}

void hashset_destroy(hashset_t set)
{
    if (set) {
        free(set->items);
    }
    free(set);
}

static int hashset_add_member(hashset_t set, void *item)
{
    size_t value = (size_t)item;
    size_t ii;

    if (value == 0 || value == 1) {
        return -1;
    }

    ii = set->mask & (prime_1 * value);

    while (set->items[ii] != 0 && set->items[ii] != 1) {
        if (set->items[ii] == value) {
            return 0;
        } else {
            /* search free slot */
            ii = set->mask & (ii + prime_2);
        }
    }
    set->nitems++;
    if (set->items[ii] == 1) {
        set->n_deleted_items--;
    }
    set->items[ii] = value;
    return 1;
}

static void maybe_rehash(hashset_t set)
{
    size_t *old_items;
    size_t old_capacity, ii;


    if (set->nitems + set->n_deleted_items >= (double)set->capacity * 0.85) {
        old_items = set->items;
        old_capacity = set->capacity;
        set->nbits++;
        set->capacity = (size_t)(1 << set->nbits);
        set->mask = set->capacity - 1;
        set->items = ( size_t *) calloc(set->capacity, sizeof(size_t));
        set->nitems = 0;
        set->n_deleted_items = 0;
        assert(set->items);
        for (ii = 0; ii < old_capacity; ii++) {
            hashset_add_member(set, (void *)old_items[ii]);
        }
        free(old_items);
    }
}

int hashset_add(hashset_t set, void *item)
{
    int rv = hashset_add_member(set, item);
    maybe_rehash(set);
    return rv;
}

int hashset_remove(hashset_t set, void *item)
{
    size_t value = (size_t)item;
    size_t ii = set->mask & (prime_1 * value);

    while (set->items[ii] != 0) {
        if (set->items[ii] == value) {
            set->items[ii] = 1;
            set->nitems--;
            set->n_deleted_items++;
            return 1;
        } else {
            ii = set->mask & (ii + prime_2);
        }
    }
    return 0;
}

int hashset_is_member(hashset_t set, void *item)
{
    size_t value = (size_t)item;
    size_t ii = set->mask & (prime_1 * value);

    while (set->items[ii] != 0) {
        if (set->items[ii] == value) {
            return 1;
        } else {
            ii = set->mask & (ii + prime_2);
        }
    }
    return 0;
}
