/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with Bohrium. 

If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __BH_MEMMAP_H
#define __BH_MEMMAP_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <map>
#include <queue>
#include <pthread.h>
#include "bh.h"
#include "bh_type.h"
#include "bh_array.h"
#include "bh_instruction.h"
#include "bh_error.h"
#include "bh_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE getpagesize()
#define BLOCK_SIZE PAGE_SIZE*2
#define PAGE_ALIGN(address) ((uintptr_t) (((uintptr_t)address) & (~((uintptr_t)(PAGE_SIZE-1)))))


long int BH_MEMMAP_OPCODE = -1;
long int BH_MEMMAP_FLUSH_OPCODE = -1;
long int BH_MEMMAP_CLOSE_OPCODE = -1;
static std::map<int, bh_base*> fids;
static std::map<bh_data_ptr, int> memmap_bases;

/** I/O Queue
 */
static std::queue<bh_view*> ioqueue;
/** I/O Queue synchronization variables
 */
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t wakeup_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  wakeup_condition = PTHREAD_COND_INITIALIZER;
static pthread_t iothread;
static int num_segfaults;

/** Initialize the memmap module.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_init_memmap(void);


/** Create a virtual memory area mapped to a file.
 *
 * @param fp bh_array containing the file path
 * @param bhtype Data type of the array
 * @param mode File access mode, string, "r+", "r", "w+"
 * @param offset int offset from where the file should be handled
 * @param shape Shape of the array which it should be translated into
 * @param order Array ordering, row or column
 * @param new_base The handler for the new base array mapped to a file
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_create_memmap(bh_instruction *instr);



/** Close a file descriptor for a file mapped array
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_close_memmap(bh_base* ary);


/** Sync the content of the filemapped array to disk.
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_flush_memmap(bh_base* ary);


/** Adds a hint to the I/O queue in form of a execution list.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_hint_memmap(bh_view* view);


/** Callback attached to the bohrium signal handler.
 *
 *  It will handle the case when the array segment is only on disk.
 *
 * @return void
 */
void bh_sighandler_memmap(unsigned long idx, uintptr_t addr);

bh_error bh_memmap_read_view(bh_view);
bh_error bh_memmap_read_base(bh_base *ary);
int bh_is_memmap(bh_base *ary);
void* bh_ioconsumer(void * args);

#ifdef __cplusplus
}
#endif

#endif
