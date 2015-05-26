#ifndef __BH_FILTER_PREPROCESSOR_H
#define __BH_FILTER_PREPROCESSOR_H

#include <bh.h>
#include "preprocessor_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT bh_error bh_filter_preprocessor_init(const char* name);
DLLEXPORT bh_error bh_filter_preprocessor_execute(bh_ir* bhir);
DLLEXPORT bh_error bh_filter_preprocessor_shutdown(void);
DLLEXPORT bh_error bh_filter_preprocessor_extmethod(const char *name, bh_opcode opcode);

#ifdef __cplusplus
}
#endif

#endif
