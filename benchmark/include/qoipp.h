#ifndef QOIPP_H_INCLUDED_
#define QOIPP_H_INCLUDED_

#define QOI_NO_STDIO
#include "qoi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void* qoipp_encode(const void* data, const qoi_desc* desc, int* out_len);
extern void* qoipp_decode(const void* data, int size, qoi_desc* desc, int channels);
extern void qoipp_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif//QOIPP_H_INCLUDED_
