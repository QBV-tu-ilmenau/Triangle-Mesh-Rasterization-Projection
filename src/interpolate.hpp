#pragma once

#include "pixel.hpp"

#include <cmath>
#include <type_traits>


namespace bmp {


    template <typename FT, typename T>
    constexpr std::common_type_t<T, FT> interpolate(FT ratio, T a, T b) noexcept {
        using type = std::common_type_t<T, FT>;
        return std::lerp(
            static_cast<type const&>(a),
            static_cast<type const&>(b),
            static_cast<type const&>(ratio));
    }

    template <typename FT, typename T>
    constexpr pixel::basic_ga<std::common_type_t<T, FT>>
        interpolate(FT ratio, pixel::basic_ga<T> const& a, pixel::basic_ga<T> const& b) noexcept {
        return {interpolate(ratio, a.g, b.g), interpolate(ratio, a.a, b.a)};
    }

    template <typename FT, typename T>
    constexpr pixel::basic_rgb<std::common_type_t<T, FT>>
        interpolate(FT ratio, pixel::basic_rgb<T> const& a, pixel::basic_rgb<T> const& b) noexcept {
        return {
            interpolate(ratio, a.r, b.r),
            interpolate(ratio, a.g, b.g),
            interpolate(ratio, a.b, b.b)};
    }

    template <typename FT, typename T>
    constexpr pixel::basic_rgba<std::common_type_t<T, FT>> interpolate(
        FT ratio,
        pixel::basic_rgba<T> const& a,
        pixel::basic_rgba<T> const& b) noexcept {
        return {
            interpolate(ratio, a.r, b.r),
            interpolate(ratio, a.g, b.g),
            interpolate(ratio, a.b, b.b),
            interpolate(ratio, a.a, b.a)};
    }


    template <typename FXT, typename FYT, typename T>
    constexpr auto interpolate_2d(FXT x_ratio, FYT y_ratio, T tl, T tr, T bl, T br) noexcept {
        return interpolate(y_ratio, interpolate(x_ratio, tl, tr), interpolate(x_ratio, bl, br));
    }


}
