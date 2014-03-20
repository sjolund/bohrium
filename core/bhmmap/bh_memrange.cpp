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

#include "bh_memrange.h"

/** Specialized Quick sort
 *
 * @param value - Value to search the arrays containing pointers
 * @param arr[] - Array of pointer pairs.
 * @param high - Size of array.
 * @param idarr - Internal id array.
 * @return - error code.
 */

void bh_quicksort(uintptr_t arr[], int low, int high, long idarr[])
{
    int i = low;
    int j = high;
    uintptr_t y = 0;
    long x = 0;

    /* compare value */
    uintptr_t z = arr[(low + high) / 2];
    /* partition */
    do
    {
        /* find member above ... */
        while(arr[i] < z) i++;

        /* find element below ... */
        while(arr[j] > z) j--;

        if(i <= j)
        {
            /* swap two elements */
            y = arr[i];
            arr[i] = arr[j];
            arr[j] = y;
            x = idarr[i];
            idarr[i] = idarr[j];
            idarr[j] = x;

            i++;
            j--;
        }
    }
    while(i <= j);

    /* recurse */
    if(low < j)
        bh_quicksort(arr, low, j, idarr);

    if(i < high)
        bh_quicksort(arr, i, high, idarr);
}
