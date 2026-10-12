#ifndef CHALLENGE_DATASETS_HPP_INCLUDED_
#define CHALLENGE_DATASETS_HPP_INCLUDED_

#define QOI_NO_STDIO
#include"qoi.h"

#include<array>
#include<algorithm>
#include<cstddef>
#include<cstdint>
#include<span>
#include<stdexcept>
#include<string_view>
#include<variant>

namespace qoi_benchmark::challenge{

template<std::size_t N>
using bytes = std::array<std::uint8_t, N>;

namespace detail{

enum : std::uint8_t{
  qoi_op_index = 0x00,
  qoi_op_diff = 0x40,
  qoi_op_luma = 0x80,
  qoi_op_run = 0xc0,
  qoi_op_rgb = 0xfe,
  qoi_op_rgba = 0xff,
};

constexpr std::uint8_t checked(int v, int min, int max){
  if(v < min || max < v)
    throw std::out_of_range("challenge dataset value out of range");
  return static_cast<std::uint8_t>(v);
}

template<std::size_t... Ns>
constexpr bytes<(Ns + ... + 0)> concat(const bytes<Ns>&... xs){
  bytes<(Ns + ... + 0)> ret = {};
  auto it = ret.begin();
  ((it = std::ranges::copy(xs, it).out), ...);
  return ret;
}

}

namespace op{

constexpr bytes<1> index(int i){
  return {static_cast<std::uint8_t>(detail::qoi_op_index | detail::checked(i, 0, 63))};
}

constexpr bytes<1> diff(int dr, int dg, int db){
  return {static_cast<std::uint8_t>(detail::qoi_op_diff | detail::checked(dr+2, 0, 3) << 4 | detail::checked(dg+2, 0, 3) << 2 | detail::checked(db+2, 0, 3))};
}

constexpr bytes<2> luma(int dg, int dr_dg, int db_dg){
  return {static_cast<std::uint8_t>(detail::qoi_op_luma | detail::checked(dg+32, 0, 63)), static_cast<std::uint8_t>(detail::checked(dr_dg+8, 0, 15) << 4 | detail::checked(db_dg+8, 0, 15))};
}

constexpr bytes<1> run(int n){
  return {static_cast<std::uint8_t>(detail::qoi_op_run | detail::checked(n-1, 0, 61))};
}

constexpr bytes<4> rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b){
  return {detail::qoi_op_rgb, r, g, b};
}

constexpr bytes<5> rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a){
  return {detail::qoi_op_rgba, r, g, b, a};
}

}

template<std::size_t... Ns>
constexpr auto qoi(std::uint32_t width, std::uint32_t height, std::uint8_t channels, const bytes<Ns>&... chunks){
  std::size_t px = 0;
  ((px += detail::qoi_op_run <= chunks[0] && chunks[0] < detail::qoi_op_rgb ? (chunks[0] & 0x3f) + 1 : 1), ...);
  if(px != static_cast<std::size_t>(width)*height)
    throw std::logic_error("chunks don't cover the image exactly");
  const bytes<14> header = {
    'q', 'o', 'i', 'f',
    static_cast<std::uint8_t>(width >> 24), static_cast<std::uint8_t>(width >> 16), static_cast<std::uint8_t>(width >> 8), static_cast<std::uint8_t>(width),
    static_cast<std::uint8_t>(height >> 24), static_cast<std::uint8_t>(height >> 16), static_cast<std::uint8_t>(height >> 8), static_cast<std::uint8_t>(height),
    detail::checked(channels, 3, 4), QOI_SRGB,
  };
  return detail::concat(header, chunks..., bytes<8>{0, 0, 0, 0, 0, 0, 0, 1});
}

template<std::size_t N>
struct raw{
  ::qoi_desc desc;
  bytes<N> pixels;
};

constexpr bytes<3> px(std::uint8_t r, std::uint8_t g, std::uint8_t b){
  return {r, g, b};
}

constexpr bytes<4> px(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a){
  return {r, g, b, a};
}

template<std::size_t Count, std::size_t N>
constexpr bytes<N*Count> repeat(const bytes<N>& x){
  bytes<N*Count> ret = {};
  for(std::size_t i = 0; i < Count; ++i)
    std::ranges::copy(x, ret.begin() + i*N);
  return ret;
}

template<std::size_t... Ns>
constexpr auto image(std::uint32_t width, std::uint32_t height, std::uint8_t channels, const bytes<Ns>&... pixels){
  if((Ns + ... + 0) != static_cast<std::size_t>(width)*height*detail::checked(channels, 3, 4))
    throw std::logic_error("pixels don't cover the image exactly");
  return raw<(Ns + ... + 0)>{{width, height, channels, QOI_SRGB}, detail::concat(pixels...)};
}

struct raw_image{
  ::qoi_desc desc;
  std::span<const std::uint8_t> pixels;
};

struct qoi_image{
  std::span<const std::uint8_t> bytes;
};

struct dataset{
  std::string_view name;
  std::variant<raw_image, qoi_image> image;
  template<std::size_t N>
  constexpr dataset(std::string_view name, const raw<N>& r):name{name}, image{raw_image{r.desc, r.pixels}}{}
  template<std::size_t N>
  constexpr dataset(std::string_view name, const bytes<N>& q):name{name}, image{qoi_image{q}}{}
};

namespace data{

// the initial pixel (0,0,0,255) gets into the index by QOI_OP_RUN
// in 3ch, alpha appears only through hash: (1,2,3) goes to slot 23 with alpha 255
inline constexpr auto rgb_run_head_then_index = qoi(6, 1, 3, op::run(2), op::rgb(10, 20, 30), op::index(53), op::rgb(1, 2, 3), op::index(23));
inline constexpr auto rgba_run_head_then_index = qoi(4, 1, 4, op::run(2), op::rgb(10, 20, 30), op::index(53));

// unused index slots hold (0,0,0,0), and QOI_OP_RGB keeps alpha
inline constexpr auto rgba_index_head_unused_slot = qoi(4, 1, 4, op::index(17), op::run(1), op::rgb(1, 2, 3), op::index(0));

// QOI_OP_INDEX also updates the index: (0,0,0,0) from slot 5 overwrites (0,0,0,128) in slot 0
inline constexpr auto rgba_index_overwrites_slot = qoi(4, 1, 4, op::rgba(0, 0, 0, 128), op::index(5), op::rgb(9, 9, 9), op::index(0));

// 3ch stream with alpha != 255: hash uses alpha 0, so (10,20,30) goes to slot 20
inline constexpr auto rgb_index_head_alpha_zero = qoi(4, 1, 3, op::index(0), op::rgb(10, 20, 30), op::diff(1, 1, 1), op::index(20));

// QOI_OP_RGBA is valid in 3ch stream: hash uses alpha 40, so (10,20,30) goes to slot 12
inline constexpr auto rgb_rgba_chunk = qoi(4, 1, 3, op::rgba(10, 20, 30, 40), op::rgb(50, 60, 70), op::index(12), op::run(1));

}

inline constexpr dataset datasets[] = {
  {"rgb_run_head_then_index", data::rgb_run_head_then_index},
  {"rgba_run_head_then_index", data::rgba_run_head_then_index},
  {"rgba_index_head_unused_slot", data::rgba_index_head_unused_slot},
  {"rgba_index_overwrites_slot", data::rgba_index_overwrites_slot},
  {"rgb_index_head_alpha_zero", data::rgb_index_head_alpha_zero},
  {"rgb_rgba_chunk", data::rgb_rgba_chunk},
};

}

#endif//CHALLENGE_DATASETS_HPP_INCLUDED_
