#ifndef IMPLEMENTATION_MACRO_HPP_INCLUDED_
#define IMPLEMENTATION_MACRO_HPP_INCLUDED_

#define CAT_I(a, b) a ## b
#define CAT(a, b) CAT_I(a, b)

#define OPTIONS_DECLARATION_(_, ident) bool CAT(run_, ident) = true;
#define OPTIONS_DECLARATION_I(_, ident)  OPTIONS_DECLARATION_(_, ident) OPTIONS_DECLARATION_II
#define OPTIONS_DECLARATION_II(_, ident) OPTIONS_DECLARATION_(_, ident) OPTIONS_DECLARATION_I
#define OPTIONS_DECLARATION_I_END
#define OPTIONS_DECLARATION_II_END
#define OPTIONS_DECLARATION(impls) CAT(OPTIONS_DECLARATION_I impls, _END)

#define OPTIONS_IMPLEMENTATION_(name, ident) \
  else if(argv == "--no" name) \
    this->CAT(run_, ident) = false;
#define OPTIONS_IMPLEMENTATION_I(name, ident)  OPTIONS_IMPLEMENTATION_(name, ident) OPTIONS_IMPLEMENTATION_II
#define OPTIONS_IMPLEMENTATION_II(name, ident) OPTIONS_IMPLEMENTATION_(name, ident) OPTIONS_IMPLEMENTATION_I
#define OPTIONS_IMPLEMENTATION_I_END
#define OPTIONS_IMPLEMENTATION_II_END
#define OPTIONS_IMPLEMENTATION(impls) CAT(OPTIONS_IMPLEMENTATION_I impls, _END)

#define LIB_DECLARATION_(_, ident) lib_t ident = {};
#define LIB_DECLARATION_I(_, ident)  LIB_DECLARATION_(_, ident) LIB_DECLARATION_II
#define LIB_DECLARATION_II(_, ident) LIB_DECLARATION_(_, ident) LIB_DECLARATION_I
#define LIB_DECLARATION_I_END
#define LIB_DECLARATION_II_END
#define LIB_DECLARATION(impls) CAT(LIB_DECLARATION_I impls, _END)

#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_, ident) this->ident += rhs.ident;
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I(_, ident)  OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_, ident) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II(_, ident) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_(_, ident) OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I_END
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION_II_END
#define OPERATOR_PLUS_EQUAL_IMPLEMENTATION(impls) CAT(OPERATOR_PLUS_EQUAL_IMPLEMENTATION_I impls, _END)

#define OUTPUT_IMPLEMENTATION_(name, ident) \
  if(printer.opt->CAT(run_, ident)) \
    output_lib(os, name, res, res.ident);
#define OUTPUT_IMPLEMENTATION_I(name, ident)  OUTPUT_IMPLEMENTATION_(name, ident) OUTPUT_IMPLEMENTATION_II
#define OUTPUT_IMPLEMENTATION_II(name, ident) OUTPUT_IMPLEMENTATION_(name, ident) OUTPUT_IMPLEMENTATION_I
#define OUTPUT_IMPLEMENTATION_I_END
#define OUTPUT_IMPLEMENTATION_II_END
#define OUTPUT_IMPLEMENTATION(impls) CAT(OUTPUT_IMPLEMENTATION_I impls, _END)

#define VERIFY_CALL_(name, ident) \
  if(opt.CAT(run_, ident)) \
    verify(name, &::CAT(ident, _encode), &::CAT(ident, _decode), &::CAT(ident, _free));
#define VERIFY_CALL_I(name, ident)  VERIFY_CALL_(name, ident) VERIFY_CALL_II
#define VERIFY_CALL_II(name, ident) VERIFY_CALL_(name, ident) VERIFY_CALL_I
#define VERIFY_CALL_I_END
#define VERIFY_CALL_II_END
#define VERIFY_CALL(impls) CAT(VERIFY_CALL_I impls, _END)

#define BENCHMARK_DECODE_CALL_(_, ident) \
  if(opt.CAT(run_, ident)) \
    BENCHMARK_DECODE(opt, result.ident.decode_time, ::CAT(ident, _decode), ::CAT(ident, _free));
#define BENCHMARK_DECODE_CALL_I(_, ident)  BENCHMARK_DECODE_CALL_(_, ident) BENCHMARK_DECODE_CALL_II
#define BENCHMARK_DECODE_CALL_II(_, ident) BENCHMARK_DECODE_CALL_(_, ident) BENCHMARK_DECODE_CALL_I
#define BENCHMARK_DECODE_CALL_I_END
#define BENCHMARK_DECODE_CALL_II_END
#define BENCHMARK_DECODE_CALL(impls) CAT(BENCHMARK_DECODE_CALL_I impls, _END)

#define BENCHMARK_ENCODE_CALL_(_, ident) \
  if(opt.CAT(run_, ident)) \
    BENCHMARK_ENCODE(opt, result.ident.encode_time, ::CAT(ident, _encode), ::CAT(ident, _free));
#define BENCHMARK_ENCODE_CALL_I(_, ident)  BENCHMARK_ENCODE_CALL_(_, ident) BENCHMARK_ENCODE_CALL_II
#define BENCHMARK_ENCODE_CALL_II(_, ident) BENCHMARK_ENCODE_CALL_(_, ident) BENCHMARK_ENCODE_CALL_I
#define BENCHMARK_ENCODE_CALL_I_END
#define BENCHMARK_ENCODE_CALL_II_END
#define BENCHMARK_ENCODE_CALL(impls) CAT(BENCHMARK_ENCODE_CALL_I impls, _END)

#define HELP_(name, _) "    --no" name " " << std::string(10-std::strlen(name), '.') << " don't execute " name "\n"
#define HELP_I(_, ident)  HELP_(_, ident) HELP_II
#define HELP_II(_, ident) HELP_(_, ident) HELP_I
#define HELP_I_END
#define HELP_II_END
#define HELP(impls) CAT(HELP_I impls, _END)

#endif//IMPLEMENTATION_MACRO_HPP_INCLUDED_
