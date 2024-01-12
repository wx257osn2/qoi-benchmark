#ifndef QOI_FU_CXX_H_INCLUDED_
#define QOI_FU_CXX_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* qoi_fu_cxx_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* qoi_fu_cxx_decode(const void* data, int size, qoi_desc* desc, int channels);
extern void qoi_fu_cxx_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//QOI_FU_CXX_H_INCLUDED_
