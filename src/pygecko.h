#ifndef _PYGECKO_H_
#define _PYGECKO_H_

#include "common/types.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for our C++ functions
void start_pygecko(void);

#ifdef __cplusplus
}
#endif

#endif
