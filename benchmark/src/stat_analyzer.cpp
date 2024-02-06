#include"implementations.hpp"
#include"implementation_macro.hpp"

#include"nlohmann/json.hpp"

#include<deque>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<string_view>
#include<unordered_map>

namespace stat_analyzer{

struct options{
  OPTIONS_DECLARATION(IMPLEMENTATIONS)
};

struct entry{
  std::string path;
  std::uint32_t width;
  std::uint32_t height;
  std::uint8_t channels;
  std::uint64_t pixels;
  struct lib_t{
    using nanosec = std::chrono::duration<double, std::nano>;
    nanosec encode_time;
    nanosec decode_time;
    lib_t& operator+=(const lib_t& rhs)noexcept{
      this->encode_time += rhs.encode_time;
      this->decode_time += rhs.decode_time;
      return *this;
    }
    lib_t& operator/=(std::size_t count)noexcept{
      this->encode_time /= count;
      this->decode_time /= count;
      return *this;
    }
  };
  lib_t qoi;
  LIB_DECLARATION(IMPLEMENTATIONS)
  struct{
    std::size_t index;
    std::size_t diff;
    std::size_t luma;
    std::size_t run;
    std::size_t rgb;
    std::size_t rgba;
  }stat;
  entry& operator+=(const entry& rhs)noexcept{
    this->pixels += rhs.pixels;
    this->qoi += rhs.qoi;
    OPERATOR_PLUS_EQUAL_IMPLEMENTATION(IMPLEMENTATIONS)
    return *this;
  }
  entry& operator/=(std::size_t count)noexcept{
    this->pixels /= count;
    this->qoi /= count;
    OPERATOR_DIV_EQUAL_IMPLEMENTATION(IMPLEMENTATIONS)
    return *this;
  }
  struct printer{
    const entry* e;
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
    static std::ostream& output_lib(std::ostream& os, std::string_view name, const entry& e, const lib_t& lib){
      const auto px = static_cast<double>(e.pixels);
      const auto empps = lib.encode_time.count() != 0 ? px / std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(lib.encode_time).count() : 0.;
      const auto dmpps = lib.decode_time.count() != 0 ? px / std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(lib.decode_time).count() : 0.;
      const auto erate = lib.encode_time.count() != 0 ? e.qoi.encode_time.count() / lib.encode_time.count() : std::numeric_limits<double>::infinity();
      const auto drate = lib.encode_time.count() != 0 ? e.qoi.decode_time.count() / lib.decode_time.count() : std::numeric_limits<double>::infinity();
      os << name << ": " << std::string(max_name_length-name.size(), ' ') << manip{8, 4} << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(lib.decode_time).count() << "    " << manip{8, 4} << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(lib.encode_time).count() << "   " << manip{11, 3} << dmpps << "   " << manip{11, 3} << empps << "   " << manip{11, 3} << drate << "   " << manip{11, 3} << erate << '\n';
      return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const printer& printer){
      const auto& res = *printer.e;
      os << std::string(max_name_length+1, ' ') << "decode ms   encode ms   decode mpps   encode mpps   decode rate   encode rate\n";
      output_lib(os, "qoi", res, res.qoi);
      OUTPUT_IMPLEMENTATION(IMPLEMENTATIONS)
      return os;
    }
  };
  printer print(const options& o)const{
    return printer{this, &o};
  }
};

static entry::lib_t parse_lib(const nlohmann::json& j){
  entry::lib_t lib;
  lib.encode_time = entry::lib_t::nanosec{j["encode_time"].template get<double>()};
  lib.decode_time = entry::lib_t::nanosec{j["decode_time"].template get<double>()};
  return lib;
}

static std::optional<std::vector<entry>> parse_json(std::istream& is, options& opt)try{
  nlohmann::json js;
  is >> js;
  std::vector<entry> entries;
  entries.reserve(js.size());
  for(auto&& j : js){
    entry e;
    e.path = j["path"].template get<std::string>();
    const auto& data = j["data"];
    e.width = data["width"].template get<std::uint32_t>();
    e.height = data["height"].template get<std::uint32_t>();
    e.channels = data["channels"].template get<std::uint8_t>();
    e.pixels = static_cast<std::size_t>(e.width) * e.height;
    e.qoi = parse_lib(data["qoi"]);
    PARSE_LIB(IMPLEMENTATIONS)
    const auto& stat = data["stat"];
    e.stat.index = stat["index"].template get<std::size_t>();
    e.stat.diff = stat["diff"].template get<std::size_t>();
    e.stat.luma = stat["luma"].template get<std::size_t>();
    e.stat.run = stat["run"].template get<std::size_t>();
    e.stat.rgb = stat["rgb"].template get<std::size_t>();
    e.stat.rgba = stat["rgba"].template get<std::size_t>();
    entries.emplace_back(std::move(e));
  }
  return entries;
}catch(std::exception& e){
  std::cout << "Error: failed to parse json\n" << e.what() << '\n';
  return std::nullopt;
}

static std::optional<std::vector<entry>> parse_json(std::string_view file, options& opt){
  if(file == "-")
    return parse_json(std::cin, opt);
  else{
    const std::filesystem::path path = file;
    if(!std::filesystem::exists(file)){
      std::cout << "Error: file is not found.\n";
      return std::nullopt;
    }
    if(std::filesystem::is_directory(path)){
      std::cout << "Error: " << file << " is directory.\n";
      return std::nullopt;
    }
    std::ifstream fs{std::filesystem::path{file}};
    return parse_json(fs, opt);
  }
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static int print(const std::vector<entry>& entries, const options& opt){
  std::deque<std::pair<entry, std::size_t>> stack;
  for(auto&& entry : entries){
    const std::filesystem::path ep = entry.path;
    if(stack.empty()){
      std::filesystem::path p = ep;
      do{
        p = p.parent_path();
        stack.push_front({{p.string()}, 0});
      }while(p.has_relative_path());
      for(const auto& [e, _] : stack)
        std::cout << "## Benchmarking " << e.path << "/*.png\n\n";
    }
    else if(stack.back().first.path != ep.parent_path()){
      auto& [back, c] = stack.back();
      const auto parent = std::next(stack.rbegin());
      parent->first += back;
      parent->second += c;
      back /= c;
      std::cout << "## Total for " << back.path << '\n'
                << back.print(opt) << std::endl;
      stack.pop_back();
      stack.push_back({{ep.parent_path().string()}, 0});
    }
    std::cout << "## " << entry.path << " size: " << entry.width << 'x' << entry.height << ", channels: " << +entry.channels << '\n'
              << entry.print(opt) << std::endl;
    stack.back().first += entry;
    ++stack.back().second;
  }
  bool flag = true;
  while(not stack.empty() && flag){
    auto& [back, c] = stack.back();
    if(stack.size() != 1){
      const auto parent = std::next(stack.rbegin());
      flag = parent->second != 0;
      parent->first += back;
      parent->second += c;
    }
    back /= c;
    std::cout << "## Total for " << back.path << '\n'
              << back.print(opt) << std::endl;
    stack.pop_back();
  }
  return EXIT_SUCCESS;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
static int defeated(const std::vector<entry>& entries, const options& opt, const std::string_view implementation){
  std::unordered_map<std::string_view, std::pair<std::size_t, std::size_t>> summary;
  for(auto&& entry : entries){
    entry::lib_t fastest = entry.qoi;
    std::string_view encode, decode;
    entry::lib_t you;
    DEFEATED(IMPLEMENTATIONS)

    const bool e = encode == implementation;
    const bool d = decode == implementation;
    if(d && e)
      continue;
    std::cout << "## " << entry.path << " size: " << entry.width << 'x' << entry.height << ", channels: " << +entry.channels << '\n'
              << entry.print(opt);
    if(!d){
      std::cout << "  Fastest decoder is " << decode << ", " << std::chrono::duration<double, std::micro>{you.decode_time}.count() << "us > " << std::chrono::duration<double, std::micro>{fastest.decode_time}.count() << "us\n";
      ++summary[decode].first;
    }
    if(!e){
      std::cout << "  Fastest encoder is " << encode << ", " << std::chrono::duration<double, std::micro>{you.encode_time}.count() << "us > " << std::chrono::duration<double, std::micro>{fastest.encode_time}.count() << "us\n";
      ++summary[encode].second;
    }
    const auto size = entry.stat.index + entry.stat.diff + entry.stat.luma + entry.stat.rgb + entry.stat.rgba + entry.stat.run;
    std::cout << "  Stat: \n"
                 "    index: " << entry.stat.index << '(' << 100. * entry.stat.index / size << "%)\n"
                 "    diff : " << entry.stat.diff << '(' << 100. * entry.stat.diff / size << "%)\n"
                 "    luma : " << entry.stat.luma << '(' << 100. * entry.stat.luma / size << "%)\n"
                 "    rgb  : " << entry.stat.rgb << '(' << 100. * entry.stat.rgb / size << "%)\n";
    if(entry.channels == 4)
      std::cout << "    rgba : " << entry.stat.rgba << '(' << 100. * entry.stat.rgba / size << "%)\n";
    std::cout << "    run  : " << entry.stat.run << '(' << 100. * entry.stat.run / size << "%)\n" << std::endl;
  }
  if(not summary.empty()){
    std::cout << "  Summary: \n";
    for(auto&& [name, de] : summary){
      std::cout << "    " << name << " is faster ";
      const auto [decode, encode] = de;
      if(decode){
        std::cout << decode << '/' << entries.size() << '(' << 100.*decode/entries.size() << "%) cases in decode";
        if(encode)
          std::cout << ", ";
      }
      if(encode)
        std::cout << encode << '/' << entries.size() << '(' << 100.*encode/entries.size() << "%) cases in encode";
      std::cout << '\n';
    }
    std::cout << std::endl;
  }
  return EXIT_SUCCESS;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static inline int help(const char* argv_0, std::ostream& os = std::cout){
  os << "Usage: " << argv_0 << " <command> [command options...] <json>\n"
        "Commands:\n"
        "    print ...... print out stats as qoibench format\n"
        "                 Options: None\n"
        "    defeated ... print out stats if defeated by other implementations\n"
        "                 Options: name .. implementation name\n"
        "Examples\n"
        "    " << argv_0 << " print stat.json\n"
        "    " << argv_0 << " defeated qoixx stat.json\n"
        "    " << argv_0 << " print - < stat.json" << std::endl;
  return EXIT_FAILURE;
}

}

int main(int argc, char** argv){
  if(argc < 3)
    return stat_analyzer::help(argv[0]);
  stat_analyzer::options opt;
  const auto command = std::string_view{argv[1]};
  if(command == "print"){
    const auto entries = parse_json(argv[2], opt);
    if(not entries)
      return stat_analyzer::help(argv[0]);
    stat_analyzer::print(*entries, opt);
  }
  else if(command == "defeated"){
    if(argc < 4){
      std::cout << "Error: not enough arguments.\n";
      return stat_analyzer::help(argv[0]);
    }
    const auto implementation = std::string_view{argv[2]};
    if(not(CHECK_NAME(IMPLEMENTATIONS))){
      std::cout << "Error: " << implementation << " is not valid implementation name.\n";
      return stat_analyzer::help(argv[0]);
    }
    const auto entries = parse_json(argv[3], opt);
    if(not entries)
      return stat_analyzer::help(argv[0]);
    stat_analyzer::defeated(*entries, opt, implementation);
  }
  else
    return stat_analyzer::help(argv[0]);
}
