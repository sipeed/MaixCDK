#ifndef __OMV_HPP
#define __OMV_HPP

#ifdef __cplusplus
// omp.h of GCC 15 contains C++ templates, include it before entering extern "C"
#include <omp.h>
extern "C" {
#endif
#include "imlib/imlib.h"

#ifdef __cplusplus
}
#endif

#endif // __OMV_HPP