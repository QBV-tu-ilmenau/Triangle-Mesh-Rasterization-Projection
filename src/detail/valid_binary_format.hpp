#pragma once

#include "../pixel.hpp"

#include <type_traits>


namespace bmp::detail {


    template <typename T, bool = ::bmp::pixel::is_pixel_type_v<T>>
    struct is_valid_binary_format
        : std::bool_constant<std::is_arithmetic_v<T> && !std::is_same_v<T, long double>> {};

    // clang-format off
    template <typename T>
    struct is_valid_binary_format<T, true>
        : std::bool_constant<std::is_arithmetic_v<typename T::value_type>
        && !std::is_same_v<typename T::value_type, long double>
        && !std::is_same_v<typename T::value_type, bool>> {};
    // clang-format on


    template <typename T>
    constexpr bool is_valid_binary_format_v = is_valid_binary_format<T>::value;


}
