#include <cphvb.h>
#include <cphvb_compute.h>

template <typename Type> cphvb_error do_jacstenc(cphvb_array *out, 
    cphvb_array *cells, cphvb_array *u, cphvb_array *l, cphvb_array *r, 
    cphvb_array *d, cphvb_float64 cons ) 
    {

    if(cphvb_data_malloc(out) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;    

    Type* out_data;
    Type* cells_data;
    Type* u_data;
    Type* l_data;
    Type* r_data;
    Type* d_data;    

    cphvb_data_get(out, (cphvb_data_ptr*) &out_data);
    cphvb_data_get(cells, (cphvb_data_ptr*) &cells_data);
    cphvb_data_get(u, (cphvb_data_ptr*) &u_data);
    cphvb_data_get(l, (cphvb_data_ptr*) &l_data);
    cphvb_data_get(r, (cphvb_data_ptr*) &r_data);
    cphvb_data_get(d, (cphvb_data_ptr*) &d_data);
    
    cphvb_intp i_out,i_cells,i_u,i_l,i_r,i_d;

    cphvb_intp M = cells->shape[0];
    cphvb_intp N = cells->shape[1];        
    
    for(cphvb_intp i = 0; i < M; i++){
        for(cphvb_intp j = 0; j < N; j++){
            
            i_out = out->start + i*out->stride[0]+j*out->stride[1];
            i_cells = cells->start + i*cells->stride[0]+j*cells->stride[1];
            i_u = u->start + i*u->stride[0]+j*u->stride[1];
            i_l = l->start + i*l->stride[0]+j*l->stride[1];
            i_r = r->start + i*r->stride[0]+j*r->stride[1];
            i_d = d->start + i*d->stride[0]+j*d->stride[1];
                        
            out_data[i_out] = (cells_data[i_cells] + u_data[i_u] + \
                    l_data[i_l] + r_data[i_r] + d_data[i_d])*cons;               
        }	
    }
    
    return CPHVB_SUCCESS;
}


/**
 * cphvb_compute_jacstenc
 *
 * Implementation of the user-defined funtion "matmul".
 * Note that we follow the function signature defined by cphvb_userfunc_impl.
 * 
 * work = (up + left + right + down) * 0.2
 * diff = abs(cells - work)
 * delta = addreduce,addreduce
 * cells = work
 * 
 */
cphvb_error cphvb_compute_jacstenc(cphvb_userfunc *arg, void* ve_arg)
{
    printf("compute jacstenc: data extracted\n");
    // cells,up,left,right,down,constant,out 
    cphvb_jacstenc_type *m_arg = (cphvb_jacstenc_type *) arg;
    
    cphvb_array     *out    = m_arg->operand[0];
    cphvb_array     *cells  = m_arg->operand[1];
    cphvb_array     *up     = m_arg->operand[2];
    cphvb_array     *left   = m_arg->operand[3];
    cphvb_array     *right  = m_arg->operand[4];
    cphvb_array     *down   = m_arg->operand[5];    
    
    cphvb_float64   cons    = m_arg->mult;
    
    
    
    //Make sure that the arrays memory are allocated.
    if(cphvb_data_malloc(out) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(cells) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(up) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(left) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(right) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;         
    if(cphvb_data_malloc(down) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;                

    switch (out->type)
    {
    	case CPHVB_INT8:
		    return do_jacstenc<cphvb_int8>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT16:
		    return do_jacstenc<cphvb_int16>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT32:
		    return do_jacstenc<cphvb_int32>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT64:
		    return do_jacstenc<cphvb_int64>(out,cells,up,left,right,down,cons);
        case CPHVB_UINT8:
		    return do_jacstenc<cphvb_uint8>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT16:
		    return do_jacstenc<cphvb_uint16>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT32:
	        return do_jacstenc<cphvb_uint32>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT64:
		    return do_jacstenc<cphvb_uint64>(out,cells,up,left,right,down,cons);
    	case CPHVB_FLOAT32:
		    return do_jacstenc<cphvb_float32>(out,cells,up,left,right,down,cons);
    	case CPHVB_FLOAT64:
		    return do_jacstenc<cphvb_float64>(out,cells,up,left,right,down,cons);
        default:
            return CPHVB_TYPE_NOT_SUPPORTED;

	}

}

