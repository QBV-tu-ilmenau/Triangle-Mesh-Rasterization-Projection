#pragma once

#include "pixel.hpp"


namespace bmp::pixel {


    template <typename T>
    struct basic_masked_pixel {
        using value_type = T;

        T v;
        bool m;

        [[nodiscard]] bool operator==(basic_masked_pixel const&) const = default;
    };


    using masked_g8 = basic_masked_pixel<std::int8_t>;
    using masked_g16 = basic_masked_pixel<std::int16_t>;
    using masked_g32 = basic_masked_pixel<std::int32_t>;
    using masked_g64 = basic_masked_pixel<std::int64_t>;
    using masked_g8u = basic_masked_pixel<std::uint8_t>;
    using masked_g16u = basic_masked_pixel<std::uint16_t>;
    using masked_g32u = basic_masked_pixel<std::uint32_t>;
    using masked_g64u = basic_masked_pixel<std::uint64_t>;

    using masked_ga8 = basic_masked_pixel<basic_ga<std::int8_t>>;
    using masked_ga16 = basic_masked_pixel<basic_ga<std::int16_t>>;
    using masked_ga32 = basic_masked_pixel<basic_ga<std::int32_t>>;
    using masked_ga64 = basic_masked_pixel<basic_ga<std::int64_t>>;
    using masked_ga8u = basic_masked_pixel<basic_ga<std::uint8_t>>;
    using masked_ga16u = basic_masked_pixel<basic_ga<std::uint16_t>>;
    using masked_ga32u = basic_masked_pixel<basic_ga<std::uint32_t>>;
    using masked_ga64u = basic_masked_pixel<basic_ga<std::uint64_t>>;

    using masked_rgb8 = basic_masked_pixel<basic_rgb<std::int8_t>>;
    using masked_rgb16 = basic_masked_pixel<basic_rgb<std::int16_t>>;
    using masked_rgb32 = basic_masked_pixel<basic_rgb<std::int32_t>>;
    using masked_rgb64 = basic_masked_pixel<basic_rgb<std::int64_t>>;
    using masked_rgb8u = basic_masked_pixel<basic_rgb<std::uint8_t>>;
    using masked_rgb16u = basic_masked_pixel<basic_rgb<std::uint16_t>>;
    using masked_rgb32u = basic_masked_pixel<basic_rgb<std::uint32_t>>;
    using masked_rgb64u = basic_masked_pixel<basic_rgb<std::uint64_t>>;

    using masked_rgba8 = basic_masked_pixel<basic_rgba<std::int8_t>>;
    using masked_rgba16 = basic_masked_pixel<basic_rgba<std::int16_t>>;
    using masked_rgba32 = basic_masked_pixel<basic_rgba<std::int32_t>>;
    using masked_rgba64 = basic_masked_pixel<basic_rgba<std::int64_t>>;
    using masked_rgba8u = basic_masked_pixel<basic_rgba<std::uint8_t>>;
    using masked_rgba16u = basic_masked_pixel<basic_rgba<std::uint16_t>>;
    using masked_rgba32u = basic_masked_pixel<basic_rgba<std::uint32_t>>;
    using masked_rgba64u = basic_masked_pixel<basic_rgba<std::uint64_t>>;


    template <typename T>
    struct is_masked_pixel_type: std::false_type {};

    template <typename T>
    struct is_masked_pixel_type<basic_masked_pixel<T>>: std::true_type {};

    template <typename T>
    constexpr bool is_masked_pixel_type_v = is_masked_pixel_type<T>::value;


}
