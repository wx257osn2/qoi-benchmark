#ifndef QOIXX_NOSIMD_H_INCLUDED_
#define QOIXX_NOSIMD_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* qoixx_nosimd_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* qoixx_nosimd_decode(const void* data, int size, qoi_desc* desc, int channels);
extern void qoixx_nosimd_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//QOIXX_NOSIMD_H_INCLUDED_
