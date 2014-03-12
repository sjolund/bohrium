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

#include <bh_memmap.h>



/** Initialize the memmap module.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_init_memmap(void)
{
    printf("bh_memmap : Init of library\n");

    return BH_SUCCESS;
}

/** Create a virtual memory area mapped to a file
 *
 * @param fp bh_array containing the file path.
 * @param bhtype Data type of the array.
 * @param mode File access mode, string, "r+", "r", "w+"
 * @param offset int where the 
 * @param shape Shape of the array which it should be translated into
 * @param order Array ordering, row or column
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_create_memmap(bh_instruction *instr)
{
    bh_view *operands = bh_inst_operands(instr);
    bh_int64* fargs = (bh_int64*)(operands[2].base->data);

    // Parse file arguments
    char* fpath = (char*)(operands[1].base->data);
    bh_int64 mode = fargs[0];
    bh_intp offset = (bh_intp)fargs[1];
    bh_int64 order = fargs[2];

    int fileflag = O_RDWR;
    // From mode create the right set of file flags
    if (mode == 0 || mode == 3)
    {
        fileflag = O_RDONLY;
    }
    else if (mode == 2) {
        fileflag |= O_TRUNC;
    }
    bh_intp size_in_bytes = bh_base_size(operands[0].base);
    // Open file with the right parameters
    int fd = open(fpath, fileflag | O_CREAT, (mode_t)0600);
    struct stat sb;

    // Stretch the file size to the correct size according to the bh_base array
    // Raise errors if things is rotten.
    if (lseek(fd, size_in_bytes-1, SEEK_SET) == -1)
    {
        fprintf(stderr, "bh_create_memmap() could not resize file:\"%s\"\n", fpath);
        return BH_ERROR;
    }
    if (write(fd, "", 1) == -1)
    {
        fprintf(stderr, "bh_create_memmap() could not write to file \"%s\"\n", fpath);
        return BH_ERROR;
    }
    // mmap virtual address space for array data
    operands[0].base->data = bh_memory_malloc(size_in_bytes);
    printf("bh_memmap: addr = %p - %p\n", operands[0].base->data, reinterpret_cast<unsigned char *>(operands[0].base->data) + size_in_bytes);
    // Attach fd and together with address space to the signal handler
    attach_signal(fd, (uintptr_t)operands[0].base->data, size_in_bytes, bh_sighandler_memmap);
    // mprotect base->data, to make sure that future access to the array will be handled by custom signal handler
    mprotect((void *)operands[0].base->data, size_in_bytes, PROT_NONE);
    fids[fd] = operands[0].base;
    printf("Finished mmap\n");
    return BH_SUCCESS;
}

/** Destroy a virtual mapping of a file to 
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_destroy_memmap(bh_base ary)
{
    return BH_SUCCESS;
}


/** Sync the content of the filemapped array to disk
 *  Guarantees that the changes to a array has been written to disk
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_sync_memmap(bh_base ary)
{
    return BH_SUCCESS;
}


/** Adds a hint to the I/O queue in form of a execution list.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_hint_memmap()
{
    return BH_SUCCESS;
}

/** Callback attached to the bohrium signal handler.
 *
 *  It will handle the case when the array segment is only on disk.
 *
 * @return void
 */
void bh_sighandler_memmap(unsigned long idx, uintptr_t addr)
{
    // First iteration; Idx will be the file handler.
    //printf("Signal handler?APAGE: %p \n", (void*)PAGE_ALIGN(addr));
    mprotect((void*)PAGE_ALIGN(addr), PAGE_SIZE, PROT_READ| PROT_WRITE);
    bh_base* base = fids[idx];
    bh_index offset = (PAGE_ALIGN(addr) - (uintptr_t)base->data);
    //printf("%p - %p = %li \n", (void*)PAGE_ALIGN(addr), base->data, offset);
    //printf("Reading from disk into: %p ' %li %i'\n", (void*)PAGE_ALIGN(addr), offset, PAGE_SIZE);
    ssize_t err = pread(idx, (void *)PAGE_ALIGN(addr), PAGE_SIZE, offset);

    //exit(-1);
}
