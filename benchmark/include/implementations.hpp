#ifndef IMPLEMENTATIONS_HPP_INCLUDED_
#define IMPLEMENTATIONS_HPP_INCLUDED_

#include"qoixx.h"
#include"qoixx_nosimd.h"
#include"qoi_rust.h"
#include"rapid-qoi.h"
#include"qoi-fu_cxx.h"
#include"qoipp.h"

#define IMPLEMENTATIONS \
/*(name str,       name ident,   pixel format of 3ch) */ \
/*("qoi",          qoi,          rgb) */ \
  ("qoixx",        qoixx,        rgb) \
  ("qoixx_nosimd", qoixx_nosimd, rgb) \
  ("qoi-rust",     qoi_rust,     rgb) \
  ("rapid-qoi",    rapid_qoi,    rgb) \
  ("qoi-fu_cxx",   qoi_fu_cxx,   argb_int) \
  ("qoipp",        qoipp,        rgb)

#endif//IMPLEMENTATIONS_HPP_INCLUDED_
