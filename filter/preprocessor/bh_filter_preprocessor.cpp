#include <stdio.h>
#include <bh.h>
#include "bh_filter_preprocessor.h"

//
// Components
//

static bh_component myself; // Myself

//Function pointers to our child.
static bh_component_iface *child;

static preprocessor_filter* preprocessor = NULL;

//
// Component interface init/execute/shutdown
//

bh_error bh_filter_preprocessor_init(const char* name)
{
    bh_error err;
    if((err = bh_component_init(&myself, name)) != BH_SUCCESS)
        return err;

    //For now, we have one child exactly
    if(myself.nchildren != 1)
    {
        fprintf(stderr, "[PREPROCESSOR-FILTER] Unexpected number of children, must be 1");
        return BH_ERROR;
    }

    //Let us initiate the child.
    child = &myself.children[0];
    if((err = child->init(child->name)) != 0)
        return err;

    try {  // Construct the preprocessor
        preprocessor = new preprocessor_filter();
    } catch (std::bad_alloc& ba) {
        fprintf(stderr, "Failed constructing Preprocessor due to allocation error.\n");
    }
    if (preprocessor == NULL) {
        return BH_ERROR;
    }
    return BH_SUCCESS;
}

bh_error bh_filter_preprocessor_shutdown(void)
{
    bh_error err = child->shutdown();
    bh_component_destroy(&myself);

    delete preprocessor;
    preprocessor = NULL;

    return err;
}

bh_error bh_filter_preprocessor_execute(bh_ir* bhir)
{
    preprocessor->preprocess(bhir);        // Run the filter
    bh_error res = child->execute(bhir); // Execute the filtered bhir
    if (res != BH_SUCCESS) {
        printf("\nerror in execution \n");
        //panic!!
    }
    // deallocate elements in base vector
    preprocessor->gc();
    
    return res;
}

bh_error bh_filter_preprocessor_extmethod(const char *name, bh_opcode opcode)
{
    return child->extmethod(name, opcode);
}

