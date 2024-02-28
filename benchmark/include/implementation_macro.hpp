#ifndef IMPLEMENTATION_MACRO_HPP_INCLUDED_
#define IMPLEMENTATION_MACRO_HPP_INCLUDED_

#include<algorithm>

#define CAT_I(a, b) a ## b
#define CAT(a, b) CAT_I(a, b)

#define OPTIONS_DECLARATION_(_0, ident, _1) bool CAT(run_, ident) = true;
#define OPTIONS_DECLARATION_I(_0, ident, _1)  OPTIONS_DECLARATION_(_0, ident, _1) OPTIONS_DECLARATION_II
#define OPTIONS_DECLARATION_II(_0, ident, _1) OPTIONS_DECLARATION_(_0, ident, _1) OPTIONS_DECLARATION_I
#define OPTIONS_DECLARATION_I_END
#define OPTIONS_DECLARATION_II_END
#define OPTIONS_DECLARATION(impls) CAT(OPTIONS_DECLARATION_I impls, _END)

#define OPTIONS_IMPLEMENTATION_(name, ident, _) \
  else if(argv == "--no" name) \
    this->CAT(run_, ident) = false;
#define OPTIONS_IMPLEMENTATION_I(name, ident, _)  OPTIONS_IMPLEMENTATION_(name, ident, _) OPTIONS_IMPLEMENTATION_II
#define OPTIONS_IMPLEMENTATION_II(name, ident, _) OPTIONS_IMPLEMENTATION_(name, ident, _) OPTIONS_IMPLEMENTATION_I
#define OPTIONS_IMPLEMENTATION_I_END
#define OPTIONS_IMPLEMENTATION_II_END
#define OPTIONS_IMPLEMENTATION(impls) CAT(OPTIONS_IMPLEMENTATION_I impls, _END)

#define LIB_DECLARATION_(_0, ident, _1) lib_t ident = {};
#define LIB_DECLARATION_I(_0, ident, _1)  LIB_DECLARATION_(_0, ident, _1) LIB_DECLARATION_II
#define LIB_DECLARATION_II(_0, ident, _1) LIB_DECLARATION_(_0, ident, _1) LIB_DECLARATION_I
#define LIB_DECLARATION_I_END
#define LIB_DECLARATION_II_END
#define LIB_DECLARATION(impls) CAT(LIB_DECLARATION_I impls, _END)

#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_0, ident, _1) this->ident += rhs.ident;
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I(_0, ident, _1)  OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_0, ident, _1) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II(_0, ident, _1) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_0, ident, _1) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I_END
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II_END
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION(impls) CAT(OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I impls, _END)

namespace qoi_benchmark::detail{

template<typename T>
struct max{
  T t;
  constexpr max operator+(const max<T>& rhs)const{
    return max{std::max(this->t, rhs.t)};
  }
};

}

#define MAX_NAME_LENGTH_(name, _0, _1) qoi_benchmark::detail::max{std::string_view{name}.size()} +
#define MAX_NAME_LENGTH_I(name, _0, _1)  MAX_NAME_LENGTH_(name, _0, _1) MAX_NAME_LENGTH_II
#define MAX_NAME_LENGTH_II(name, _0, _1) MAX_NAME_LENGTH_(name, _0, _1) MAX_NAME_LENGTH_I
#define MAX_NAME_LENGTH_I_END qoi_benchmark::detail::max<std::size_t>{3/*qoi*/}
#define MAX_NAME_LENGTH_II_END qoi_benchmark::detail::max<std::size_t>{3/*qoi*/}
#define MAX_NAME_LENGTH(impls) (CAT(MAX_NAME_LENGTH_I impls, _END)).t

#define OUTPUT_IMPLEMENTATION_(name, ident, _) \
  if(printer.opt->CAT(run_, ident)) \
    output_lib(os, name, res, res.ident);
#define OUTPUT_IMPLEMENTATION_I(name, ident, _)  OUTPUT_IMPLEMENTATION_(name, ident, _) OUTPUT_IMPLEMENTATION_II
#define OUTPUT_IMPLEMENTATION_II(name, ident, _) OUTPUT_IMPLEMENTATION_(name, ident, _) OUTPUT_IMPLEMENTATION_I
#define OUTPUT_IMPLEMENTATION_I_END
#define OUTPUT_IMPLEMENTATION_II_END
#define OUTPUT_IMPLEMENTATION(impls) CAT(OUTPUT_IMPLEMENTATION_I impls, _END)

#define MAKE_ENTRY_(name, ident, _) \
  ret[name] = make_entry(result.ident);
#define MAKE_ENTRY_I(name, ident, _)  MAKE_ENTRY_(name, ident, _) MAKE_ENTRY_II
#define MAKE_ENTRY_II(name, ident, _) MAKE_ENTRY_(name, ident, _) MAKE_ENTRY_I
#define MAKE_ENTRY_I_END
#define MAKE_ENTRY_II_END
#define MAKE_ENTRY(impls) CAT(MAKE_ENTRY_I impls, _END)

#define VERIFY_CALL_(name, ident, pixel_format) \
  { \
    std::atomic_ref<bool> valid{result.ident.valid}; \
    if(opt.CAT(run_, ident) && valid && !qoi_benchmark::CAT(verify_, pixel_format)(name, p, &::CAT(ident, _encode), &::CAT(ident, _decode), &::CAT(ident, _free), CAT(PTR_OF_, pixel_format), desc, encoded_qoi.get(), out_len, opt.allow_broken_implementation)) \
      valid = false; \
  }
#define VERIFY_CALL_I(name, ident, pixel_format)  VERIFY_CALL_(name, ident, pixel_format) VERIFY_CALL_II
#define VERIFY_CALL_II(name, ident, pixel_format) VERIFY_CALL_(name, ident, pixel_format) VERIFY_CALL_I
#define VERIFY_CALL_I_END
#define VERIFY_CALL_II_END
#define VERIFY_CALL(impls) CAT(VERIFY_CALL_I impls, _END)

#define BENCHMARK_DECODE_CALL_(_0, ident, _1) \
  if(opt.CAT(run_, ident)) \
    BENCHMARK_DECODE(opt, result.ident.decode_time, ::CAT(ident, _decode), ::CAT(ident, _free));
#define BENCHMARK_DECODE_CALL_I(_0, ident, _1)  BENCHMARK_DECODE_CALL_(_0, ident, _1) BENCHMARK_DECODE_CALL_II
#define BENCHMARK_DECODE_CALL_II(_0, ident, _1) BENCHMARK_DECODE_CALL_(_0, ident, _1) BENCHMARK_DECODE_CALL_I
#define BENCHMARK_DECODE_CALL_I_END
#define BENCHMARK_DECODE_CALL_II_END
#define BENCHMARK_DECODE_CALL(impls) CAT(BENCHMARK_DECODE_CALL_I impls, _END)

#define BENCHMARK_ENCODE_CALL_(_, ident, pixel_format) \
  if(opt.CAT(run_, ident)){ \
    BENCHMARK_ENCODE(opt, result.ident.encode_time, ::CAT(ident, _encode), ::CAT(ident, _free), pixel_format); \
  }
#define BENCHMARK_ENCODE_CALL_I(_, ident, pixel_format)  BENCHMARK_ENCODE_CALL_(_, ident, pixel_format) BENCHMARK_ENCODE_CALL_II
#define BENCHMARK_ENCODE_CALL_II(_, ident, pixel_format) BENCHMARK_ENCODE_CALL_(_, ident, pixel_format) BENCHMARK_ENCODE_CALL_I
#define BENCHMARK_ENCODE_CALL_I_END
#define BENCHMARK_ENCODE_CALL_II_END
#define BENCHMARK_ENCODE_CALL(impls) CAT(BENCHMARK_ENCODE_CALL_I impls, _END)

#define HELP_(name, _0, _1) "    --no" name " " << std::string(std::max(benchmark_result_t::printer::max_name_length+1, static_cast<std::size_t>(13))-std::strlen(name), '.') << " don't execute " name "\n"
#define HELP_I(name, _0, _1)  HELP_(name, _0, _1) HELP_II
#define HELP_II(name, _0, _1) HELP_(name, _0, _1) HELP_I
#define HELP_I_END
#define HELP_II_END
#define HELP(impls) CAT(HELP_I impls, _END)

#define CHECK_NAME_(name, _0, _1) std::string_view{name} == implementation
#define CHECK_NAME_I(name, _0, _1)  CHECK_NAME_(name, _0, _1) || CHECK_NAME_II
#define CHECK_NAME_II(name, _0, _1) CHECK_NAME_(name, _0, _1) || CHECK_NAME_I
#define CHECK_NAME_I_END false
#define CHECK_NAME_II_END false
#define CHECK_NAME(impls) CAT(CHECK_NAME_I impls, _END)

#define PARSE_LIB_(name, ident, _) \
  e.ident = parse_lib(data[name]); \
  opt.CAT(run_, ident) = e.ident.encode_time.count() != 0 && e.ident.decode_time.count() != 0;
#define PARSE_LIB_I(name, ident, _)  PARSE_LIB_(name, ident, _) PARSE_LIB_II
#define PARSE_LIB_II(name, ident, _) PARSE_LIB_(name, ident, _) PARSE_LIB_I
#define PARSE_LIB_I_END
#define PARSE_LIB_II_END
#define PARSE_LIB(impls) CAT(PARSE_LIB_I impls, _END)

#define OPERATOR_DIV_EQUAL_IMPLEMENTATION_(_0, ident, _1) this->ident /= count;
#define OPERATOR_DIV_EQUAL_IMPLEMENTATION_I(_0, ident, _1)  OPERATOR_DIV_EQUAL_IMPLEMENTATION_(_0, ident, _1) OPERATOR_DIV_EQUAL_IMPLEMENTATION_II
#define OPERATOR_DIV_EQUAL_IMPLEMENTATION_II(_0, ident, _1) OPERATOR_DIV_EQUAL_IMPLEMENTATION_(_0, ident, _1) OPERATOR_DIV_EQUAL_IMPLEMENTATION_I
#define OPERATOR_DIV_EQUAL_IMPLEMENTATION_I_END
#define OPERATOR_DIV_EQUAL_IMPLEMENTATION_II_END
#define OPERATOR_DIV_EQUAL_IMPLEMENTATION(impls) CAT(OPERATOR_DIV_EQUAL_IMPLEMENTATION_I impls, _END)

#define DEFEATED_(name, ident, _) \
  if(entry.ident.encode_time.count() > 0 and fastest.encode_time > entry.ident.encode_time){ \
    fastest.encode_time = entry.ident.encode_time; \
    encode = name; \
  } \
  if(entry.ident.encode_time.count() > 0 and fastest.decode_time > entry.ident.decode_time){ \
    fastest.decode_time = entry.ident.decode_time; \
    decode = name; \
  } \
  if(name == implementation) \
    you = entry.ident;
#define DEFEATED_I(name, ident, _)  DEFEATED_(name, ident, _) DEFEATED_II
#define DEFEATED_II(name, ident, _) DEFEATED_(name, ident, _) DEFEATED_I
#define DEFEATED_I_END
#define DEFEATED_II_END
#define DEFEATED(impls) CAT(DEFEATED_I impls, _END)

#endif//IMPLEMENTATION_MACRO_HPP_INCLUDED_
