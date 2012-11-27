// jit_codegenerated_kernel_functions.cpp

#include "stdlib.h"
#include "cphvb_array.h"
#include "cphvb_type.h"
#include "cphvb.h"

#define K_NARRAYS 3
#define K_NCONSTANTS 0
#define K_OUT_NDIM 2
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#if _WIN32
#include <float.h>
#define cphvb_isnan(x) (_isnan(x))
#define cphvb_isinf(x) (!_isnan(x) || !_finite(x))
#else
#define cphvb_isnan(x) (std::isnan(x))
#define cphvb_isinf(x) (std::isinf(x))
#endif
#define DEG_CIR 360.0
#define DEG_RAD (M_PI / (DEG_CIR / 2.0))
#define RAD_DEG ((DEG_CIR / 2.0) / M_PI)

// A+B
void kernel_func_1899990464_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = (*(OffA1 + DA1) + *(OffA2 + DA2));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

// C
void kernel_func_4265622974_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = ((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

// D
void kernel_func_1625152366_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
cphvb_float32* DA4 = (cphvb_float32*) cphvb_base_array( as[4])->data;
cphvb_index OffA4;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
	OffA4 = as[4]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
		OffA4 += as[4]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = (((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3)) + *(OffA4 + DA4));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
		OffA4 += as[4]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }


void kernel_func_3286243741_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
cphvb_float32* DA4 = (cphvb_float32*) cphvb_base_array( as[4])->data;
cphvb_index OffA4;
cphvb_float32* DA5 = (cphvb_float32*) cphvb_base_array( as[5])->data;
cphvb_index OffA5;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
	OffA4 = as[4]->start;
	OffA5 = as[5]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
		OffA4 += as[4]->stride[j] * coord[j];
		OffA5 += as[5]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = ((((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3)) + *(OffA4 + DA4)) + *(OffA5 + DA5));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
		OffA4 += as[4]->stride[last_dim];
		OffA5 += as[5]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

void kernel_func_1904711455_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
    cphvb_index last_dim = as[0]->ndim-1;
    cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
    cphvb_index ec = 0;
    cphvb_index coord[2];
    memset(coord, 0, 2 * sizeof(cphvb_index));
    cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
    cphvb_index OffA0;
    cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
    cphvb_index OffA1;
    int j=0, i=0;
    if (skip>0) {                                // Create coord based on skip
        while(ec<skip) {
            ec += as[0]->shape[last_dim];
            for(j = (last_dim-1); j >= 0; --j) {
                coord[j]++;
            }
        }
    }
    while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
        // Compute offset based on coord
        for(j=0; j<last_dim; ++j) {
    OffA0 += as[0]->stride[j] * coord[j];
    OffA1 += as[1]->stride[j] * coord[j];
        }
        for(j=0; j < as[0]->shape[last_dim]; j++ ) {
            // computation code
            *(OffA0 + DA0) = *(OffA1 + DA1);
            printf("%f. ",(cphvb_float32) *(OffA0 + DA0));
    OffA0 += as[0]->stride[last_dim];
    OffA1 += as[1]->stride[last_dim];
        }
        ec += as[0]->shape[last_dim];
        for(j = last_dim-1; j >= 0; --j) {
            coord[j]++;
            if (coord[j] < as[0]->shape[j]) {
                break;
            } else {
                coord[j] = 0;
            }
        }
    }
}
