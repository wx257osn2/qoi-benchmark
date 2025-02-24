#include"qoipp.hpp"
#include<cassert>
#include<stack>

typedef struct{
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
}qoi_desc;

namespace {

thread_local std::stack<qoipp::ByteVec> qoipp_results;

}

extern "C"{

void* qoipp_encode(const void* data, const qoi_desc* desc, int* out_len){
  const qoipp::ImageDesc d = {
    .m_width = desc->width,
    .m_height = desc->height,
    .m_channels = static_cast<qoipp::Channels>(desc->channels),
    .m_colorspace = static_cast<qoipp::Colorspace>(desc->colorspace),
  };
  const auto size = static_cast<std::size_t>(desc->width) * desc->height * desc->channels;
  try{
    auto enc = qoipp::encode(data, size, d);
    const auto ptr = enc.data();
    *out_len = static_cast<int>(enc.size());
    qoipp_results.push(std::move(enc));
    return ptr;
  }catch(...){
    return nullptr;
  }
}

void* qoipp_decode(const void* data, int size, qoi_desc* desc, int channels){
  try{
    auto [dec, ppdesc] = qoipp::decode(data, static_cast<std::size_t>(size), {static_cast<qoipp::Channels>(channels)});
    const auto ptr = dec.data();
    desc->width = ppdesc.m_width;
    desc->height = ppdesc.m_height;
    desc->channels = static_cast<unsigned char>(ppdesc.m_channels);
    desc->colorspace = static_cast<unsigned char>(ppdesc.m_colorspace);
    qoipp_results.push(std::move(dec));
    return ptr;
  }catch(...){
    return nullptr;
  }
}

void qoipp_free([[maybe_unused]] void* ptr){
  assert(ptr == qoipp_results.top().data());
  qoipp_results.pop();
}

}
