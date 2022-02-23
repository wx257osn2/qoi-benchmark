#ifndef QOI_RUST_H_INCLUDED_
#define QOI_RUST_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* qoi_rust_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* qoi_rust_decode(const void* data, int size, qoi_desc* desc, int);
extern void qoi_rust_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//QOI_RUST_H_INCLUDED_
