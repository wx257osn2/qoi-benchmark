#include"qoixx.hpp"
#include<memory>
#include<cstdint>
#include<cstddef>
#include<concepts>

#if __has_include("qoixx_driver_qoi_malloc.hpp")
#include"qoixx_driver_qoi_malloc.hpp"
#elif not defined(QOI_MALLOC)
#include<cstdlib>
#define QOI_MALLOC(sz) std::malloc(sz)
#define QOI_FREE(p)    std::free(p)
#endif

typedef struct{
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
}qoi_desc;

namespace qoixx_driver{

namespace{

struct deleter{
  constexpr deleter() noexcept = default;
  template<typename T>
  void operator()(T* ptr)const{QOI_FREE(ptr);}
};

struct pointer_and_size{
  std::unique_ptr<std::uint8_t[], deleter> ptr;
  int size;
};

}

}

namespace qoixx{

template<>
struct container_operator<qoixx_driver::pointer_and_size>{
  using target_type = qoixx_driver::pointer_and_size;
  static inline target_type construct(std::size_t size){
    target_type t{std::unique_ptr<std::uint8_t[], qoixx_driver::deleter>{static_cast<std::uint8_t*>(QOI_MALLOC(size))}, 0};
    return t;
  }
  struct pusher{
    static constexpr bool is_contiguous = true;
    target_type* t;
    inline void push(std::uint8_t x)noexcept{
      t->ptr[t->size++] = x;
    }
    template<typename T>
    requires std::unsigned_integral<T> && (sizeof(T) != 1)
    inline void push(T t)noexcept{
      this->push(static_cast<std::uint8_t>(t));
    }
    inline target_type finalize()noexcept{
      return std::move(*t);
    }
    inline std::uint8_t* raw_pointer()noexcept{
      return t->ptr.get()+t->size;
    }
    inline void advance(std::size_t n)noexcept{
      t->size += static_cast<int>(n);
    }
  };
  static constexpr pusher create_pusher(target_type& t)noexcept{
    return {&t};
  }
  struct puller{
    static constexpr bool is_contiguous = true;
    const std::uint8_t* t;
    inline std::uint8_t pull()noexcept{
      return *t++;
    }
    inline const std::uint8_t* raw_pointer()noexcept{
      return t;
    }
    inline void advance(std::size_t n)noexcept{
      t += n;
    }
  };
  static puller create_puller(const target_type& t)noexcept{
    return {t.ptr.get()};
  }
  static inline std::size_t size(const target_type& t)noexcept{
    return static_cast<std::size_t>(t.size);
  }
  static bool valid(const target_type& t)noexcept{
    return t.ptr != nullptr;
  }
};

}

extern "C"{

void* qoixx_encode(const void* data, const qoi_desc* desc, int* out_len){
  const qoixx::qoi::desc d = {
    .width = desc->width,
    .height = desc->height,
    .channels = desc->channels,
    .colorspace = static_cast<qoixx::qoi::colorspace>(desc->colorspace),
  };
  const auto size = static_cast<std::size_t>(d.width) * d.height * d.channels;
  try{
    auto enc = qoixx::qoi::encode<qoixx_driver::pointer_and_size>(static_cast<const std::uint8_t*>(data), size, d);
    *out_len = enc.size;
    return enc.ptr.release();
  }catch(...){
    return nullptr;
  }
}

void* qoixx_decode(const void* data, int size, qoi_desc* desc, int channels){
  try{
    auto [dec, xxdesc] = qoixx::qoi::decode<qoixx_driver::pointer_and_size>(static_cast<const std::uint8_t*>(data), static_cast<std::size_t>(size), static_cast<std::uint8_t>(channels));
    desc->width = xxdesc.width;
    desc->height = xxdesc.height;
    desc->channels = xxdesc.channels;
    desc->colorspace = static_cast<unsigned char>(xxdesc.colorspace);
    return dec.ptr.release();
  }catch(...){
    return nullptr;
  }
}

void qoixx_free(void* ptr){
  QOI_FREE(ptr);
}

}
