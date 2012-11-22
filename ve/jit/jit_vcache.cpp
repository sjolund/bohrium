

#include "jit_vcache.h"
using namespace std;

cphvb_error jit_vcache_free( cphvb_array* array ) {
    cphvb_array* base;
    cphvb_intp nelements, bytes;

    base = cphvb_base_array( array ); 
    if (base->data != NULL) {
        nelements   = cphvb_nelements(base->ndim, base->shape);
        bytes       = nelements * cphvb_type_size(base->type);
        
		//DEBUG_PRINT("Deallocate=%p\n", base->data);
        cphvb_vcache_insert( base->data, bytes );
		base->data = NULL;
    }
    array = NULL;

    return CPHVB_SUCCESS;
}

/**
 * Requires that the mcache have been initialized.
 * uses the functionality of cphvb_mcache. This function just peals of
 * the instruction related code compared to the cphvb_mcache_malloc().
 **/
cphvb_error jit_vcache_malloc( cphvb_array* array ) {
    cphvb_array* base;
    cphvb_intp nops, i, nelements, bytes;

    base = cphvb_base_array( array );
    if (base->data == NULL) {

        nelements   = cphvb_nelements(base->ndim, base->shape);
        bytes       = nelements * cphvb_type_size(base->type);

        //DEBUG_PRINT("Reuse or allocate...\n");
        base->data = cphvb_vcache_find( bytes );
        if (base->data == NULL) {
            if (cphvb_data_malloc(array) != CPHVB_SUCCESS) {                
                return CPHVB_OUT_OF_MEMORY;         // EXIT
            }                                   
            //DEBUG_PRINT("Allocated=%p\n", base->data);
        } else {
            //DEBUG_PRINT("Reusing=%p.\n", base->data);
        }
    }            
    return CPHVB_SUCCESS;
}
