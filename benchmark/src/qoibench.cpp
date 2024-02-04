#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include"stb_image.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include"nlohmann/json.hpp"

#define QOI_NO_STDIO
#include"qoi.h"

#include"implementation_macro.hpp"
#include"implementations.hpp"

#include"suppress_optimization.hpp"
#include"thread_pool.hpp"

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
#include<charconv>
#include<variant>

namespace qoi_benchmark{

struct options{
  bool warmup = true;
  bool verify = true;
  bool encode = true;
  bool decode = true;
  bool recurse = true;
  bool only_totals = false;
  bool allow_broken_implementation = false;
  OPTIONS_DECLARATION(IMPLEMENTATIONS)
  unsigned runs;
  unsigned threads = 1;
  std::filesystem::path stat_path;
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
    else if(argv.starts_with("--threads=")){
      const auto result = std::from_chars(argv.data()+std::strlen("--threads="), argv.data()+argv.size(), this->threads);
      if(result.ptr != argv.data()+argv.size() || result.ec != std::errc{}){
        if(result.ec == std::errc::invalid_argument)
          throw std::runtime_error(std::string{"invalid argument: "} + argv.data());
        if(result.ec == std::errc::result_out_of_range)
          throw std::range_error(std::string{"threads count out of range: "} + (argv.data()+std::strlen("--threads=")));
        throw std::runtime_error(std::string{"invalid threads count: "} + (argv.data()+std::strlen("--threads=")));
      }
    }
    else if(argv.starts_with("--stat_path=")){
      argv.remove_prefix(std::strlen("--stat_path="));
      this->stat_path = argv;
    }
    OPTIONS_IMPLEMENTATION(IMPLEMENTATIONS)
    else
      return false;
    return true;
  }
};

namespace detail{

template<typename T>
struct max{
  T t;
  constexpr max operator+(const max<T>& rhs)const{
    return max{std::max(this->t, rhs.t)};
  }
};

}

struct qoi_stat{
  std::uint64_t rgba = 0;
  std::uint64_t rgb = 0;
  std::uint64_t index = 0;
  std::uint64_t diff = 0;
  std::uint64_t luma = 0;
  std::uint64_t run[62] = {};
  explicit constexpr qoi_stat() = default;
  explicit constexpr qoi_stat(const std::uint8_t* data, std::size_t n){
    enum{
      qoi_index = 0x00,
      qoi_diff = 0x40,
      qoi_luma = 0x80,
      qoi_run = 0xc0,
      qoi_rgb = 0xfe,
      qoi_rgba = 0xff
    };
    for(std::size_t i = 0; i < n; ++i){
      const auto x = data[i];
      if(x == qoi_rgba){
        ++this->rgba;
        i += 4;
        continue;
      }
      else if(x == qoi_rgb){
        ++this->rgb;
        i += 3;
        continue;
      }
      const auto xm = x & 0b11000000;
      if(xm == qoi_index)
        ++this->index;
      if(xm == qoi_diff)
        ++this->diff;
      if(xm == qoi_luma){
        ++this->luma;
        ++i;
      }
      if(xm == qoi_run)
        ++this->run[x & 0b00111111];
    }
  }
  nlohmann::json to_json()const{
    return nlohmann::json{
      {"rgba", this->rgba},
      {"rgb", this->rgb},
      {"index", this->index},
      {"diff", this->diff},
      {"luma", this->luma},
      {"run", std::accumulate(std::cbegin(this->run), std::cend(this->run), 0)},
    };
  }
};

using nanosec = std::chrono::duration<double, std::nano>;
struct benchmark_result_t{
  struct lib_t{
    nanosec encode_time;
    nanosec decode_time;
    bool valid = true;
    lib_t& operator+=(const lib_t& rhs)noexcept{
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
  lib_t qoi = {};
  LIB_DECLARATION(IMPLEMENTATIONS)
  benchmark_result_t():count{0}, px{0}{}
  benchmark_result_t(const ::qoi_desc& dc):count{1}, px{static_cast<std::size_t>(dc.width)*dc.height}, w{dc.width}, h{dc.height}, c{dc.channels}{}
  benchmark_result_t& operator+=(const benchmark_result_t& rhs)noexcept{
    this->count += rhs.count;
    this->px += rhs.px;
    this->qoi += rhs.qoi;
    OPERATOR_PLUS_EQUAL_IMPLEMENTATION(IMPLEMENTATIONS)
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
    static constexpr std::size_t max_name_length = MAX_NAME_LENGTH(IMPLEMENTATIONS);
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
      OUTPUT_IMPLEMENTATION(IMPLEMENTATIONS)
      return os;
    }
  };
  printer print(const options& opt)const{
    return printer{this, &opt};
  }
};

static inline nlohmann::json make_entry(const benchmark_result_t::lib_t& lib){
  return nlohmann::json{
    {"encode_time", lib.encode_time.count()},
    {"decode_time", lib.decode_time.count()},
  };
}

static inline nlohmann::json make_entry(const benchmark_result_t& result, const qoi_stat& stat){
  nlohmann::json ret = {
    {"width", result.w},
    {"height", result.h},
    {"channels", result.c},
    {"stat", stat.to_json()}
  };
  ret["qoi"] = make_entry(result.qoi);
  MAKE_ENTRY(IMPLEMENTATIONS)
  return ret;
}

static inline bool compare(const ::qoi_desc& lhs, const ::qoi_desc& rhs){
  return lhs.width == rhs.width && lhs.height == rhs.height && lhs.channels == rhs.channels && lhs.colorspace == rhs.colorspace;
}

template<bool False = false>
static std::vector<std::uint8_t> convert_to_argb_int(const std::uint8_t* pixels, unsigned int num, unsigned char channels){
  std::vector<std::uint8_t> data(num*4);
  if(channels == 4){
    for(auto i : std::views::iota(0u, num))
      if constexpr(std::endian::native == std::endian::little){
        // rgba -> bgra
        data[i*4]   = pixels[i*4+2];
        data[i*4+1] = pixels[i*4+1];
        data[i*4+2] = pixels[i*4+0];
        data[i*4+3] = pixels[i*4+3];
      }
      else if constexpr(std::endian::native == std::endian::big){
        // rgba -> argb
        data[i*4]   = pixels[i*4+3];
        data[i*4+1] = pixels[i*4+0];
        data[i*4+2] = pixels[i*4+1];
        data[i*4+3] = pixels[i*4+2];
      }
      else
        static_assert(False, "unsupported endian");
    return data;
  }
  for(auto i : std::views::iota(0u, num))
    if constexpr(std::endian::native == std::endian::little){
      // rgb -> bgra
      data[i*4]   = pixels[i*3+2];
      data[i*4+1] = pixels[i*3+1];
      data[i*4+2] = pixels[i*3];
      data[i*4+3] = 0xff;
    }
    else if constexpr(std::endian::native == std::endian::big){
      // rgb -> argb
      data[i*4]   = 0xff;
      data[i*4+1] = pixels[i*3];
      data[i*4+2] = pixels[i*3+1];
      data[i*4+3] = pixels[i*3+2];
    }
    else
      static_assert(False, "unsupported endian");
  return data;
}

static inline bool verify_rgb(const char* name, const std::filesystem::path& p, void* (*encoder)(const void*, const qoi_desc*, int*), void* (*decoder)(const void*, int, qoi_desc*, int), void(*free)(void*), const std::uint8_t* pixels, const qoi_desc& desc, const std::uint8_t* encoded, int encoded_size, bool shelve)try{
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
  return true;
}catch(std::exception& e){
  if(shelve){
    std::cout << e.what() << std::endl;
    return false;
  }
  else
    throw;
}

static inline bool verify_argb_int(const char* name, const std::filesystem::path& p, void* (*encoder)(const void*, const qoi_desc*, int*), void* (*decoder)(const void*, int, qoi_desc*, int), void(*free)(void*), const std::uint8_t* pixels, const qoi_desc& desc, const std::uint8_t* encoded, int encoded_size, bool shelve)try{
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
    const auto converted = convert_to_argb_int(pixs.get(), desc.width * desc.height, desc.channels);
    if(!compare(desc, dc) || std::memcmp(pixels, converted.data(), dc.width*dc.height*dc.channels) != 0)
      throw std::runtime_error(std::string{name} + " encoder pixel mismatch for " + p.string());
  }
  {// encoder -> decoder == pixels
    ::qoi_desc dc;
    const auto pixs = std::unique_ptr<std::uint8_t[], decltype(free)>{static_cast<std::uint8_t*>(decoder(enc.get(), size, &dc, desc.channels)), free};
    if(!compare(desc, dc) || std::memcmp(pixels, pixs.get(), desc.width*desc.height*desc.channels) != 0)
      throw std::runtime_error(std::string{name} + " roundtrip pixel mismatch for " + p.string());
  }
  return true;
}catch(std::exception& e){
  if(shelve){
    std::cout << e.what() << std::endl;
    return false;
  }
  else
    throw;
}

template<typename Deleter>
using unique_ptr_for_benchmark = std::unique_ptr<std::uint8_t[], Deleter>;
#define BENCHMARK(opt, result, preamble, ...) \
do{ \
  std::chrono::nanoseconds time = {}; \
  for(unsigned i = opt.warmup ? 0u : 1u; i <= opt.runs; ++i){ \
    preamble \
    const auto start = std::chrono::high_resolution_clock::now(); \
    __VA_ARGS__ \
    detail::suppress_optimization(pixs.get()); \
    const auto end = std::chrono::high_resolution_clock::now(); \
    if(i > 0) \
      time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start); \
  } \
  result = std::chrono::duration_cast<nanosec>(time)/opt.runs; \
}while(0)

#define BENCHMARK_DECODE(opt, result, f, d) BENCHMARK(opt, result, ::qoi_desc dc;, const unique_ptr_for_benchmark<decltype(&d)> pixs(static_cast<std::uint8_t*>(f(encoded_qoi.get(), out_len, &dc, channels)), &d);)
#define BENCHMARK_ENCODE(opt, result, f, d, pixel_format) BENCHMARK(opt, result, int size;, const unique_ptr_for_benchmark<decltype(&d)> pixs(static_cast<std::uint8_t*>(f(CAT(PTR_OF_, pixel_format), &desc, &size)), &d);)

static inline std::pair<benchmark_result_t, qoi_stat> benchmark_image(const std::filesystem::path& p, const options& opt){
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

  const auto argb_int_pixels = convert_to_argb_int(pixels.get(), desc.width * desc.height, desc.channels);
#define PTR_OF_rgb pixels.get()
#define PTR_OF_argb_int argb_int_pixels.data()

  benchmark_result_t result{desc};
  if(opt.verify){
    VERIFY_CALL(IMPLEMENTATIONS)
  }

  if(opt.decode){
    BENCHMARK_DECODE(opt, result.qoi.decode_time, ::qoi_decode, ::free);
    BENCHMARK_DECODE_CALL(IMPLEMENTATIONS)
  }

  if(opt.encode){
    BENCHMARK_ENCODE(opt, result.qoi.encode_time, ::qoi_encode, ::free, rgb);
    BENCHMARK_ENCODE_CALL(IMPLEMENTATIONS)
  }

  if(not opt.stat_path.empty())
    return std::make_pair(result, qoi_stat{encoded_qoi.get(), static_cast<std::size_t>(out_len)});
  else
    return std::make_pair(result, qoi_stat{});
}

using thread_pool_type = thread_pool<std::pair<benchmark_result_t, qoi_stat>, std::pair<benchmark_result_t, qoi_stat>(*)(const std::filesystem::path&, const options& opt), std::filesystem::path, const options&>;
static inline std::pair<benchmark_result_t, nlohmann::json> benchmark_directory(const std::filesystem::path& path, const options& opt, std::optional<thread_pool_type>& threads){
  if(!std::filesystem::is_directory(path))
    throw std::runtime_error(path.string() + " is not a directory");

  benchmark_result_t results = {};
  nlohmann::json json = nlohmann::json::array();

  if(opt.recurse)
    for(const auto& x : std::ranges::subrange{std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{}})
      if(x.is_directory()){
        const auto [res, j] = benchmark_directory(x, opt, threads);
        results += res;
        json.insert(json.cend(), j.cbegin(), j.cend());
      }

  if(threads){
    std::vector<std::pair<std::future<std::pair<benchmark_result_t, qoi_stat>>, std::string>> futures;
    bool first = true;
    for(const auto& x : std::ranges::subrange{std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{}}){
      auto xp = x.path();
      if(x.is_directory() || xp.extension() != ".png")
        continue;
      if(std::exchange(first, false))
        std::cout << "## Benchmarking " << path.string() << "/*.png -- " << opt.runs << " runs\n\n";

      futures.emplace_back(threads->push(benchmark_image, xp, opt), xp.string());
    }
    for(auto&& [f, xp] : futures){
      const auto [result, stat] = f.get();
      if(!opt.only_totals)
        std::cout << "## " << xp << " size: " << result.w << 'x' << result.h << ", channels: " << +result.c << '\n'
                  << result.print(opt) << std::endl;

      if(not opt.stat_path.empty())
        json.push_back(nlohmann::json{{"path", xp}, {"data", make_entry(result, stat)}});
      results += result;
    }
  }
  else{
    bool first = true;
    for(const auto& x : std::ranges::subrange{std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{}}){
      auto xp = x.path();
      if(x.is_directory() || xp.extension() != ".png")
        continue;
      if(std::exchange(first, false))
        std::cout << "## Benchmarking " << path.string() << "/*.png -- " << opt.runs << " runs\n\n";

      const auto [result, stat] = benchmark_image(xp, opt);
      if(!opt.only_totals)
        std::cout << "## " << xp.string() << " size: " << result.w << 'x' << result.h << ", channels: " << +result.c << '\n'
                  << result.print(opt) << std::endl;

      if(not opt.stat_path.empty())
        json.push_back(nlohmann::json{{"path", xp.string()}, {"data", make_entry(result, stat)}});
      results += result;
    }
  }

  if(results.count > 0)
    std::cout << "## Total for " << path << '\n'
              << results.print(opt) << std::endl;

  return std::make_pair(results, json);
}

static inline int help(const char* argv_0, std::ostream& os = std::cout){
  os << "Usage: " << argv_0 << " <iterations> <directory> [options...]\n"
        "Options:\n"
        "    --nowarmup ....... don't perform a warmup run\n"
        "    --noverify ....... don't verify qoi roundtrip\n"
        "    --noencode ....... don't run encoders\n"
        "    --nodecode ....... don't run decoders\n"
        "    --norecurse ...... don't descend into directories\n"
        "    --onlytotals ..... don't print individual image results\n"
        "    --nohalt ......... don't stop if some implementation fail validation\n"
        "    --threads=n ...... multithread execution, where n is threads count (default = 1)\n"
        "                       when n = 0 or a value greater than the hardware-supported concurrency,\n"
        "                       the maximum number of parallel threads supported by the hardware\n"
        "                       will be launched.\n"
        "    --stat_path=path . output statistic json to path.\n"
        HELP(IMPLEMENTATIONS)
        "Examples\n"
        "    ./" << argv_0 << " 10 images/textures/\n"
        "    ./" << argv_0 << " 1 images/textures/ --nowarmup" << std::endl;
  return EXIT_FAILURE;
}

}

int main(int argc, char** argv)try{
  if(argc < 3)
    return qoi_benchmark::help(argv[0]);
  qoi_benchmark::options opt = {};
  for(int i = 3; i < argc; ++i)
    if(!opt.parse_option(argv[i])){
      std::cout << "Unknown option " << argv[i] << '\n';
      return qoi_benchmark::help(argv[0]);
    }

  const auto runs = std::stoi(argv[1]);
  if(runs <= 0){
    std::cout << "Invalid number of runs " << runs << std::endl;
    return EXIT_FAILURE;
  }
  opt.runs = static_cast<unsigned>(runs);

  std::optional<qoi_benchmark::thread_pool_type> threads = std::nullopt;
  if(opt.threads != 1)
    threads.emplace(opt.threads == 0 ? std::max(1u, std::thread::hardware_concurrency()) : opt.threads);

  const auto [result, json] = qoi_benchmark::benchmark_directory(argv[2], opt, threads);
  if(result.count > 0)
    std::cout << "# Grand total for " << argv[2] << '\n'
              << result.print(opt) << std::endl;
  else
    std::cout << "No images found in " << argv[2] << std::endl;

  if(not opt.stat_path.empty()){
    std::ofstream file(opt.stat_path);
    file << json << std::endl;
  }
}catch(const std::runtime_error& e){
  std::cout << e.what() << std::endl;
  return EXIT_FAILURE;
}
