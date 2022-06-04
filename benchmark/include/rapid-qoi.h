#ifndef RAPID_QOI_H_INCLUDED_
#define RAPID_QOI_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* rapid_qoi_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* rapid_qoi_decode(const void* data, int size, qoi_desc* desc, int);
extern void rapid_qoi_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//RAPID_QOI_H_INCLUDED_
