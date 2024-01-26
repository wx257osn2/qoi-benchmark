#ifndef SUPPRESS_OPTIMIZATION_HPP_INCLUDED_
#define SUPPRESS_OPTIMIZATION_HPP_INCLUDED_

#include<type_traits>

namespace qoi_benchmark{

namespace detail{

template<typename T>
concept available_on_register = std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(T*);

template<typename T>
requires available_on_register<T>
[[gnu::always_inline]] inline void suppress_optimization(const T& v)noexcept{
  asm volatile("" : : "r,m"(v) : "memory");
}

template<typename T>
requires (not available_on_register<T>)
[[gnu::always_inline]] inline void suppress_optimization(const T& v)noexcept{
  asm volatile("" : : "m"(v) : "memory");
}

template<typename T>
requires available_on_register<T>
[[gnu::always_inline]] inline void suppress_optimization(T& v)noexcept{
  asm volatile("" : "+m,r"(v) : : "memory");
}

template<typename T>
requires (not available_on_register<T>)
[[gnu::always_inline]] inline void suppress_optimization(T& v)noexcept{
  asm volatile("" : "+m"(v) : : "memory");
}

template<typename T>
requires (not std::is_reference_v<T> and available_on_register<T>)
[[gnu::always_inline]] inline void suppress_optimization(T&& v)noexcept{
  asm volatile("" : "+m,r"(v) : : "memory");
}

template<typename T>
requires (not std::is_reference_v<T> and not available_on_register<T>)
[[gnu::always_inline]] inline void suppress_optimization(T&& v)noexcept{
  asm volatile("" : "+m"(v) : : "memory");
}

}

}

#endif//SUPPRESS_OPTIMIZATION_HPP_INCLUDED_
