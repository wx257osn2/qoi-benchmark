#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#elif defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(push)
#pragma warning(disable: 4820 4365 4505)
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include"stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb_image_write.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#define QOI_NO_STDIO
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(push)
#pragma warning(disable: 4820 4365 4244 4242)
#endif
#include"qoi.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include"qoixx.h"
#include"qoi_rust.h"

#include<chrono>
#include<iostream>
#include<filesystem>
#include<string_view>
#include<ranges>
#include<fstream>
#include<cstddef>
#include<vector>
#include<memory>
#include<utility>
#include<cstdint>
#include<iomanip>
#include<cstring>

struct options{
  bool warmup = true;
  bool verify = true;
  bool encode = true;
  bool decode = true;
  bool recurse = true;
  bool only_totals = false;
  bool allow_broken_implementation = false;
  bool run_qoixx = true;
  bool run_qoi_rust = true;
  unsigned runs;
  bool parse_option(std::string_view argv){
    if(argv == "--nowarmup")
      this->warmup = false;
    else if(argv == "--noverify")
      this->verify = false;
    else if(argv == "--noencode")
      this->encode = false;
    else if(argv == "--nodecode")
      this->decode = false;
    else if(argv == "--norecurse")
      this->recurse = false;
    else if(argv == "--onlytotals")
      this->only_totals = true;
    else if(argv == "--nohalt")
      this->allow_broken_implementation = true;
    else if(argv == "--noqoixx")
      this->run_qoixx = false;
    else if(argv == "--noqoi-rust")
      this->run_qoi_rust = false;
    else
      return false;
    return true;
  }
};

struct benchmark_result_t{
  struct lib_t{
    std::size_t size;
    std::chrono::duration<double, std::nano> encode_time;
    std::chrono::duration<double, std::nano> decode_time;
    bool valid = true;
    lib_t& operator+=(const lib_t& rhs)noexcept{
      size += rhs.size;
      encode_time += rhs.encode_time;
      decode_time += rhs.decode_time;
      valid = valid && rhs.valid;
      return *this;
    }
  };
  std::size_t count;
  std::size_t px;
  std::uint32_t w, h;
  std::uint8_t c;
  lib_t qoi = {}, qoixx = {}, qoi_rust = {};
  benchmark_result_t():count{0}, px{0}{}
  benchmark_result_t(const ::qoi_desc& dc):count{1}, px{static_cast<std::size_t>(dc.width)*dc.height}, w{dc.width}, h{dc.height}, c{dc.channels}{}
  benchmark_result_t& operator+=(const benchmark_result_t& rhs)noexcept{
    this->count += rhs.count;
    this->px += rhs.px;
    this->qoi += rhs.qoi;
    this->qoixx += rhs.qoixx;
    this->qoi_rust += rhs.qoi_rust;
    return *this;
  }
  struct printer{
    const benchmark_result_t* result;
    const options* opt;
    struct manip{
      int w;
      int prec = 0;
      friend std::ostream& operator<<(std::ostream& os, const manip& m){
        os << std::fixed << std::setw(m.w);
        if(m.prec != 0)
          os << std::setprecision(m.prec);
        return os;
      }
    };
    static constexpr std::size_t max_name_length = 9;
    static std::ostream& output_lib(std::ostream& os, std::string_view name, const benchmark_result_t& res, const lib_t& lib){
      if(!lib.valid)
        return os << name << " outputs invalid content\n";
      const auto px = static_cast<double>(res.px) / res.count;
      const auto etime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(lib.encode_time) / res.count;
      const auto dtime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(lib.decode_time) / res.count;
      const auto empps = lib.encode_time.count() != 0 ? px / std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(etime).count() : 0.;
      const auto dmpps = lib.decode_time.count() != 0 ? px / std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(dtime).count() : 0.;
      const auto erate = lib.encode_time.count() != 0 ? static_cast<double>(res.qoi.encode_time.count()) / lib.encode_time.count() : std::numeric_limits<double>::infinity();
      const auto drate = lib.encode_time.count() != 0 ? static_cast<double>(res.qoi.decode_time.count()) / lib.decode_time.count() : std::numeric_limits<double>::infinity();
      os << name << ": " << std::string(max_name_length-name.size(), ' ') << manip{8, 4} << dtime.count() << "    " << manip{8, 4} << etime.count() << "      " << manip{8, 3} << dmpps << "      " << manip{8, 3} << empps << "      " << manip{8, 3} << drate << "      " << manip{8, 3} << erate << '\n';
      return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const printer& printer){
      const auto& res = *printer.result;
      os << std::string(max_name_length+1, ' ') << "decode ms   encode ms   decode mpps   encode mpps   decode rate   encode rate\n";
      output_lib(os, "qoi", res, res.qoi);
      if(printer.opt->run_qoixx)
        output_lib(os, "qoixx", res, res.qoixx);
      if(printer.opt->run_qoi_rust)
        output_lib(os, "qoi_rust", res, res.qoi_rust);
      return os;
    }
  };
  printer print(const options& opt)const{
    return printer{this, &opt};
  }
};

static inline bool compare(const ::qoi_desc& lhs, const ::qoi_desc& rhs){
  return lhs.width == rhs.width && lhs.height == rhs.height && lhs.channels == rhs.channels && lhs.colorspace == rhs.colorspace;
}

static inline void verify(const char* name, const std::filesystem::path& p, void* (*encoder)(const void*, const qoi_desc*, int*), void* (*decoder)(const void*, int, qoi_desc*, int), void(*free)(void*), const std::uint8_t* pixels, const qoi_desc& desc, const std::uint8_t* encoded, int encoded_size, bool shelve)try{
  {// qoi.encode -> decoder == pixels
    qoi_desc dc;
    const auto pixs = std::unique_ptr<std::uint8_t[], decltype(free)>{static_cast<std::uint8_t*>(decoder(encoded, encoded_size, &dc, desc.channels)), free};
    if(!compare(desc, dc) || pixs == nullptr || std::memcmp(pixels, pixs.get(), desc.width*desc.height*desc.channels) != 0)
      throw std::runtime_error(std::string{name} + " decoder pixel mismatch for " + p.string());
  }
  int size;
  const auto enc = std::unique_ptr<std::uint8_t[], decltype(free)>{static_cast<std::uint8_t*>(encoder(pixels, &desc, &size)), free};
  if(enc == nullptr || size != encoded_size)
    throw std::runtime_error(std::string{name} + " encoder can't encode " + p.string());
  {// encoder -> qoi.decode == pixels
    ::qoi_desc dc;
    const auto pixs = std::unique_ptr<std::uint8_t[], decltype(&::free)>{static_cast<std::uint8_t*>(::qoi_decode(enc.get(), size, &dc, desc.channels)), &::free};
    if(!compare(desc, dc) || std::memcmp(pixels, pixs.get(), dc.width*dc.height*dc.channels) != 0)
      throw std::runtime_error(std::string{name} + " encoder pixel mismatch for " + p.string());
  }
  {// encoder -> decoder == pixels
    ::qoi_desc dc;
    const auto pixs = std::unique_ptr<std::uint8_t[], decltype(free)>{static_cast<std::uint8_t*>(decoder(enc.get(), size, &dc, desc.channels)), free};
    if(!compare(desc, dc) || std::memcmp(pixels, pixs.get(), desc.width*desc.height*desc.channels) != 0)
      throw std::runtime_error(std::string{name} + " roundtrip pixel mismatch for " + p.string());
  }
}catch(std::exception& e){
  if(shelve)
    std::cout << e.what() << std::endl;
  else
    throw;
}

#define BENCHMARK(opt, result, preamble, ...) \
do{ \
  std::chrono::nanoseconds time = {}; \
  for(unsigned i = opt.warmup ? 0u : 1u; i <= opt.runs; ++i){ \
    preamble \
    const auto start = std::chrono::high_resolution_clock::now(); \
    __VA_ARGS__ \
    const auto end = std::chrono::high_resolution_clock::now(); \
    if(i > 0) \
      time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start); \
  } \
  result = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(time)/opt.runs; \
}while(0)

#define BENCHMARK_DECODE(opt, result, f, d) BENCHMARK(opt, result, ::qoi_desc dc;, const std::unique_ptr<std::uint8_t[], decltype(&d)> pixs{static_cast<std::uint8_t*>(f(encoded_qoi.get(), out_len, &dc, channels)), &d};)
#define BENCHMARK_ENCODE(opt, result, f, d) BENCHMARK(opt, result, int size;, const std::unique_ptr<std::uint8_t[], decltype(&d)> pixs{static_cast<std::uint8_t*>(f(pixels.get(), &desc, &size)), &d};)

static inline benchmark_result_t benchmark_image(const std::filesystem::path& p, const options& opt){
  int w, h, channels;

  if(!stbi_info(p.string().c_str(), &w, &h, &channels))
    throw std::runtime_error("Error decoding header " + p.string());

  if(channels != 3)
    channels = 4;

  const ::qoi_desc desc = {
    .width = static_cast<unsigned int>(w),
    .height = static_cast<unsigned int>(h),
    .channels = static_cast<unsigned char>(channels),
    .colorspace = QOI_SRGB,
  };

  const auto pixels = std::unique_ptr<::stbi_uc[], decltype(&::stbi_image_free)>{::stbi_load(p.string().c_str(), &w, &h, nullptr, channels), &::stbi_image_free};
  int out_len;
  const auto encoded_qoi = std::unique_ptr<std::uint8_t[], decltype(&::free)>{static_cast<std::uint8_t*>(::qoi_encode(pixels.get(), &desc, &out_len)), &::free};

  if(!pixels)
    throw std::runtime_error("Error decoding " + p.string());

  if(opt.verify){
    if(opt.run_qoixx)
      verify("qoixx", p, &::qoixx_encode, &qoixx_decode, &qoixx_free, pixels.get(), desc, encoded_qoi.get(), out_len, opt.allow_broken_implementation);
    if(opt.run_qoi_rust)
      verify("qoi_rust", p, &::qoi_rust_encode, &qoi_rust_decode, &qoi_rust_free, pixels.get(), desc, encoded_qoi.get(), out_len, opt.allow_broken_implementation);
  }

  benchmark_result_t result{desc};
  if(opt.decode){
    BENCHMARK_DECODE(opt, result.qoi.decode_time, ::qoi_decode, ::free);
    if(opt.run_qoixx)
      BENCHMARK_DECODE(opt, result.qoixx.decode_time, ::qoixx_decode, ::qoixx_free);
    if(opt.run_qoi_rust)
      BENCHMARK_DECODE(opt, result.qoi_rust.decode_time, ::qoi_rust_decode, ::qoi_rust_free);
  }

  if(opt.encode){
    BENCHMARK_ENCODE(opt, result.qoi.encode_time, ::qoi_encode, ::free);
    if(opt.run_qoixx)
      BENCHMARK_ENCODE(opt, result.qoixx.encode_time, ::qoixx_encode, ::qoixx_free);
    if(opt.run_qoi_rust)
      BENCHMARK_ENCODE(opt, result.qoi_rust.encode_time, ::qoi_rust_encode, ::qoi_rust_free);
  }

  return result;
}

static inline benchmark_result_t benchmark_directory(const std::filesystem::path& path, const options& opt){
  if(!std::filesystem::is_directory(path))
    throw std::runtime_error(path.string() + " is not a directory");

  benchmark_result_t results = {};

  if(opt.recurse)
    for(const auto& x : std::ranges::subrange{std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{}})
      if(x.is_directory())
        results += benchmark_directory(x, opt);

  bool first = true;
  for(const auto& x : std::ranges::subrange{std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{}}){
    auto xp = x.path();
    if(x.is_directory() || xp.extension() != ".png")
      continue;
    if(std::exchange(first, false))
      std::cout << "## Benchmarking " << path.string() << "/*.png -- " << opt.runs << " runs\n\n";

    const auto result = benchmark_image(xp, opt);
    if(!opt.only_totals)
      std::cout << "## " << xp.string() << " size: " << result.w << 'x' << result.h << ", channels: " << +result.c << '\n'
                << result.print(opt) << std::endl;

    results += result;
  }

  if(results.count > 0)
    std::cout << "## Total for " << path << '\n'
              << results.print(opt) << std::endl;

  return results;
}

static inline int help(const char* argv_0, std::ostream& os = std::cout){
  os << "Usage: " << argv_0 << " <iterations> <directory> [options...]\n"
        "Options:\n"
        "    --nowarmup ... don't perform a warmup run\n"
        "    --noverify ... don't verify qoi roundtrip\n"
        "    --noencode ... don't run encoders\n"
        "    --nodecode ... don't run decoders\n"
        "    --norecurse .. don't descend into directories\n"
        "    --onlytotals . don't print individual image results\n"
        "Examples\n"
        "    ./" << argv_0 << " 10 images/textures/\n"
        "    ./" << argv_0 << " 1 images/textures/ --nowarmup" << std::endl;
  return EXIT_FAILURE;
}

int main(int argc, char** argv)try{
  if(argc < 3)
    return help(argv[0]);
  options opt = {};
  for(int i = 3; i < argc; ++i)
    if(!opt.parse_option(argv[i])){
      std::cout << "Unknown option " << argv[i] << '\n';
      return help(argv[0]);
    }

  const auto runs = std::stoi(argv[1]);
  if(runs <= 0){
    std::cout << "Invalid number of runs " << runs << std::endl;
    return EXIT_FAILURE;
  }
  opt.runs = static_cast<unsigned>(runs);

  const auto result = benchmark_directory(argv[2], opt);
  if(result.count > 0)
    std::cout << "# Grand total for " << argv[2] << '\n'
              << result.print(opt) << std::endl;
  else
    std::cout << "No images found in " << argv[2] << std::endl;
}catch(const std::runtime_error& e){
  std::cout << e.what() << std::endl;
  return EXIT_FAILURE;
}
