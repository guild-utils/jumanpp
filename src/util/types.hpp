//
// Created by Arseny Tolmachev on 2017/02/17.
//

#ifndef JUMANPP_TYPES_HPP
#define JUMANPP_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace jumanpp {

// signed types
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

// usigned types
using u8 = std::uint8_t;
using u16 = std::int16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using size_t = std::size_t;
using ptrdiff_t = std::ptrdiff_t;
using uint32 = u32;
using uint8 = u8;
}  // namespace jumanpp

#endif  // JUMANPP_TYPES_HPP
