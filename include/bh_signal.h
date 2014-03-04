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



#ifndef __BH_SIGNAL_H
#define __BH_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct mspace
{
    unsigned long idx;
    uintptr_t start;
    uintptr_t end;
    void (*callback)(unsigned long, uintptr_t);
};
typedef struct mspace mSpace;

static long mspaceid;
static int init;
static int spacesize;
static int spacesmax;
static int used;

mSpace *mspaces;
static long *idssorted;
uintptr_t *ptssorted;

static void sighandler(int signal_number, siginfo_t *info, void *context);

static void expandarrays(void);

static int addspace(mSpace m);
static int removespace(int idx);

void quicksort(uintptr_t arr[], int low, int high, long idarr[]);

int search(uintptr_t start);
static int searchinterval(uintptr_t start, uintptr_t end);
static int binarysearch(uintptr_t value, uintptr_t arr[], int high);

static int findid(signed long id, void (*callback));

int init_signal(void);

int attach_signal(signed long idx,
                  uintptr_t start,
                  long int size,
                  void (*callback)(unsigned long, uintptr_t));

int detach_signal(signed long idx, void (*callback));

#ifdef __cplusplus
}
#endif

#endif
