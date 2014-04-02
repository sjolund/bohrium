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

/** I/O Queue
 */

struct ioqueue_item {
    bh_view     view;
    int         esize;
    bh_index    nelem;
    bh_data_ptr data;
    ioqueue_item() : esize(0), data(0) {}
    ioqueue_item(const bh_view& v) : view(v), esize(bh_type_size(v.base->type)), nelem(v.base->nelem), data(v.base->data) {
    }
    bool is_empty() const { return data == 0; }
};
static std::queue<ioqueue_item> ioqueue;
/** I/O Queue synchronization variables
 */
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  queue_not_empty_condition = PTHREAD_COND_INITIALIZER;
pthread_t iothread;
int num_segfaults;
int num_segfaults_reads;
int num_prefetch;

pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<uint32_t, bool> pages_map;


pthread_mutex_t fids_mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<int, bh_base*> fids;
std::map<bh_data_ptr, int> memmap_bases;

void ioqueue_add_item(const ioqueue_item& item);

/** Initialize the memmap module.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_init_memmap(void)
{
    //printf("bh_memmap : \033[1mInit of library\033[0m \n");
    num_segfaults = 0;
    pthread_create( &iothread, NULL, &bh_ioconsumer, NULL);
    return BH_SUCCESS;
}

void bh_memmap_shutdown()
{
    ioqueue_add_item(ioqueue_item());
    pthread_join(iothread, NULL);
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
    //bh_intp offset = (bh_intp)fargs[1];
    //bh_int64 order = fargs[2];

    int fileflag = O_RDWR;
    // From mode create the right set of file flags
    if (mode == 0 || mode == 3)
    {
        fileflag = O_RDONLY;
    }
    else if (mode == 2) {
        fileflag |= O_TRUNC;
    }
    fileflag |= O_DIRECT;
    bh_intp size_in_bytes = bh_base_size(operands[0].base);
    // Open file with the right parameters
    int fd = open(fpath, fileflag | O_CREAT, (mode_t)0600);

    if (mode != 0)
    {
        // Stretch the file size to the correct size according to the bh_base array
        // Raise errors if things are rotten.
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
    }
    // mmap virtual address space for array data
    //bh_intp errv = bh_memory_free(operands[0].base->data, size_in_bytes);
    operands[0].base->data = bh_memory_malloc(size_in_bytes);
    // Attach fd and together with address space to the signal handler
    attach_signal(fd, (uintptr_t)operands[0].base->data, size_in_bytes, bh_sighandler_memmap);
    // -rw@base->data, to make sure that future access to the array will be handled by custom signal handler
    mprotect((void *)operands[0].base->data, size_in_bytes, PROT_NONE);
    pthread_mutex_lock(&fids_mutex);
    fids[fd] = operands[0].base;
    pthread_mutex_unlock(&fids_mutex);
    memmap_bases[operands[0].base->data] = fd;
    return BH_SUCCESS;
}

/** Close a file descriptor for a file mapped array
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_close_memmap(bh_base* ary)
{
    int fid = memmap_bases.at(ary->data);

    memmap_bases.erase(ary->data);
    pthread_mutex_lock(&fids_mutex);
    fids.erase(fid);
    pthread_mutex_unlock(&fids_mutex);

    if (close(fid) == -1)
    {
        fprintf(stderr, "bh_memmap could not close file handler\n");
        return BH_ERROR;
    }

    return BH_SUCCESS;
}


/** Flush the content of the filemapped array to disk
 *  Guarantees that the changes to a array has been written to disk
 *
 * @param ary bh_array file mapped array.
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_flush_memmap(bh_base* ary)
{
    //printf("IN FLUSH\n");
    //printf("FLUSH| %p->%p\n", ary, ary->data);
    int fd = memmap_bases.at(ary->data);
    printf("Flush fd(%i)\n", fd);
    if (pwrite(fd, ary->data, bh_base_size(ary), 0) == -1)
    {
        fprintf(stderr, "bh_memmap could not write content to file, error message: %s\n", strerror(errno));
        return BH_ERROR;
    }
    return BH_SUCCESS;
}

void ioqueue_add_item(const ioqueue_item& item)
{
    pthread_mutex_lock( &queue_mutex );
    // This part should maybe be done in the read_view function so that
    // it can check if a particular page has already been read into memory.
    if (!ioqueue.empty())
    {
        ioqueue_item back = ioqueue.back();
        if (&back.view == &item.view)
        {
            pthread_mutex_unlock( &queue_mutex );
            return;
        }
    }
    //printf("bh_hint: inserting element in I/O Queue: %p\n", &item.view);

    ioqueue.push(item);
    pthread_cond_signal( &queue_not_empty_condition );

    pthread_mutex_unlock( &queue_mutex );
}


/** Adds a hint to the I/O queue in form of a execution list.
 *
 * @return Error code (BH_SUCCESS, BH_OUT_OF_MEMORY)
 */
bh_error bh_hint_memmap(bh_view* view)
{
    ioqueue_add_item(*view);
    return BH_SUCCESS;
}


bh_error bh_read_page(bh_index page, bh_index filesize, bh_data_ptr data_p, int fd)
{
    bh_index offset = PAGE_ALIGN(page) - (uintptr_t)data_p;
    size_t pagesize = BLOCK_SIZE;
    if (filesize < (BLOCK_SIZE+offset)){
        pagesize = filesize - offset;
    }
    void *buffer = mmap(0, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    ssize_t err = pread(fd, buffer, pagesize, offset);
    if(mremap(buffer, pagesize, pagesize, MREMAP_FIXED|MREMAP_MAYMOVE, (void *)PAGE_ALIGN(page)) == MAP_FAILED)
    {
        printf("MREMAP FAILED\n");
    }

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
    __sync_fetch_and_add(&num_segfaults, 1);
    // First iteration; Idx will be the file handler.
    pthread_mutex_lock(&fids_mutex);
    bh_base* base = fids[idx];
    bh_index filesize = bh_base_size(base);
    bh_index offset = PAGE_ALIGN(addr) - (uintptr_t)base->data;
    pthread_mutex_unlock(&fids_mutex);
    pthread_mutex_lock(&read_mutex);
    if (pages_map.count(PAGE_ALIGN(addr)) == 0)
    {
        bh_read_page(addr, filesize, base->data, idx);
        pages_map[addr] = true;
        __sync_fetch_and_add(&num_segfaults_reads, 1);
    }
    pthread_mutex_unlock(&read_mutex);
}


/** Function for determining if a base array is associated with a 
 *  memmory mapped file.
 *
 *  @return 1=true 0=false
 */
int bh_is_memmap(bh_base *ary)
{
    return memmap_bases.count(ary->data);
}

/** Interprets a bh_view
 *  and reads the page sized block into memory from disk
 *
 */
bh_error bh_memmap_read_view(const ioqueue_item& item)
{

    //printf("bh_memmap_read_view\n");
    int fd = memmap_bases.at(item.data);
    bh_index filesize = item.nelem * item.esize;
    bh_index counters[BH_MAXDIM];
    memset(counters, 0, sizeof(counters));
    std::vector<bh_index> pages;
    while(counters[0] < item.view.shape[0])
    {
        bh_index index = item.view.start;
        for (int dim=0; dim < item.view.ndim; dim++)
        {
            index += counters[dim] * item.view.stride[dim];
        }
        bh_index page = PAGE_ALIGN((bh_index) item.data+index*item.esize);
        if ((pages.empty() || pages.back() != page) )
        {
            pthread_mutex_lock(&read_mutex);
            if (pages_map.count(page) == 0)
            {
                pages_map[page] = true;
                bh_read_page(page, filesize, item.data, fd);
                __sync_fetch_and_add(&num_prefetch, 1);
            }

            pthread_mutex_unlock(&read_mutex);
            pages.push_back(page);
        }

        for (int dim=item.view.ndim-1; dim >= 0 ; dim--)
        {
            counters[dim] += 1;
            if (counters[dim] < item.view.shape[dim])
            {
                break;
            }
            if (dim != 0)
            {
                counters[dim] = 0;
            }

        }
    }
    //for (std::vector<bh_index>::const_iterator it = pages.begin(), end = pages.end(); it != end; ++it) {
    //    printf("%lx\n", *it);
    //}

    //printf("dim->%li | start-> %li\n", item.view.ndim, item.view.start);
    //for (int i=0; i < item.view.ndim; i++){
    //    printf("shape->%li | stride->%li \n", item.view.shape[i], item.view.stride[i]);
    //}
    return BH_SUCCESS;
}


/** Will read an entire file mapped base array into memory
 *
 *  Is primarily used for BH_SYNC's
 */
bh_error bh_memmap_read_base(bh_base *ary)
{

    printf("READING ALL FROM FILE!!! AUCH..\n");
    int fid = memmap_bases.at(ary->data);
    bh_index size = bh_base_size(ary);
    mprotect(ary->data, size, PROT_WRITE);
    ssize_t err = pread(fid, ary->data, size, 0);
    mprotect(ary->data, size, PROT_READ| PROT_WRITE);
    return BH_SUCCESS;
}


void* bh_ioconsumer(void * args)
{
    for (;;){
        pthread_mutex_lock( &queue_mutex );
        while (ioqueue.empty())
        {
            pthread_cond_wait( &queue_not_empty_condition, &queue_mutex );
        }
        ioqueue_item item = ioqueue.front();
        ioqueue.pop();
        pthread_mutex_unlock( &queue_mutex );
        if (item.is_empty()) {
            break;
        }
        bh_memmap_read_view( item );
    }
    return 0;
}


void bh_memmap_stats()
{
    //printf("bh_memmap stats:\n");
    //printf("    # of segfaults:  %i(%i) \n", num_segfaults, num_segfaults_reads);
    //printf("    # of prefecthes: %i \n", num_prefetch);
    printf("%i, %i, %i\n", num_segfaults, num_segfaults_reads, num_prefetch);
}
