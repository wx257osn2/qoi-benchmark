#include"QOI.hpp"
#include<cassert>
#include<stack>

typedef struct{
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
}qoi_desc;

template<typename Accessor, typename Accessor::type Member>
struct access_private_member{
  friend typename Accessor::type get(Accessor){
    return Member;
  }
};

struct encoder_accessor{
  using type = std::shared_ptr<uint8_t[]> QOIEncoder::*;
  friend type get(encoder_accessor);
};

struct decoder_accessor{
  using type = std::shared_ptr<int[]> QOIDecoder::*;
  friend type get(decoder_accessor);
};

template struct access_private_member<encoder_accessor, &QOIEncoder::encoded>;
template struct access_private_member<decoder_accessor, &QOIDecoder::pixels>;

namespace{

thread_local std::stack<std::shared_ptr<void>> qoi_fu_results;

}

extern "C"{

void* qoi_fu_cxx_encode(const void* data, const qoi_desc* desc, int* out_len){
  QOIEncoder encoder;
  if(!encoder.encode(desc->width, desc->height, static_cast<const int*>(data), desc->channels == 4, desc->colorspace == 1))
    return nullptr;
  *out_len = encoder.getEncodedSize();
  auto result = std::move(encoder.*get(encoder_accessor{}));
  const auto ptr = result.get();
  qoi_fu_results.push(std::move(result));
  return ptr;
}

void* qoi_fu_cxx_decode(const void* data, int size, qoi_desc* desc, int){
  QOIDecoder decoder;
  if(!decoder.decode(static_cast<const uint8_t*>(data), size))
    return nullptr;
  desc->width = decoder.getWidth();
  desc->height = decoder.getHeight();
  desc->channels = decoder.hasAlpha() ? 4 : 3;
  desc->colorspace = decoder.isLinearColorspace() ? 1 : 0;
  auto result = std::move(decoder.*get(decoder_accessor{}));
  const auto ptr = result.get();
  qoi_fu_results.push(std::move(result));
  return ptr;
}

void qoi_fu_cxx_free([[maybe_unused]] void* ptr){
  assert(ptr == qoi_fu_results.top().get());
  qoi_fu_results.pop();
}

}
