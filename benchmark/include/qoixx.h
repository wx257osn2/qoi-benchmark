#ifndef QOIXX_H_INCLUDED_
#define QOIXX_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* qoixx_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* qoixx_decode(const void* data, int size, qoi_desc* desc, int channels);
extern void qoixx_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//QOIXX_H_INCLUDED_
