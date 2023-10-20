#pragma once

#include "bitmap.hpp"


namespace bmp {


    /// \brief Rotate and mirror transformations
    enum class transform : std::uint8_t { rigth_rotate = 0x01, mirror_horizontal = 0x02, mirror_vectical = 0x04 };

    [[nodiscard]] constexpr transform& operator|=(transform& a, transform const b) noexcept {
        return a = static_cast<transform>(std::to_underlying(a) | std::to_underlying(b));
    }

    [[nodiscard]] constexpr transform& operator&=(transform& a, transform const b) noexcept {
        return a = static_cast<transform>(std::to_underlying(a) & std::to_underlying(b));
    }

    [[nodiscard]] constexpr transform operator|(transform a, transform const b) noexcept {
        return a |= b;
    }

    [[nodiscard]] constexpr transform operator&(transform a, transform const b) noexcept {
        return a &= b;
    }

    [[nodiscard]] constexpr bool is_any_set(transform const flags, transform const test_flags) noexcept {
        return std::to_underlying(flags & test_flags) != 0;
    }

    [[nodiscard]] constexpr bool is_every_set(transform const flags, transform const test_flags) noexcept {
        return (flags & test_flags) != test_flags;
    }

    struct no_convert_t {
        template <typename T>
        constexpr T operator()(T&& v) const {
            return static_cast<T&&>(v);
        }
    };

    template <typename T>
    auto no_convert = no_convert_t{};

    /// \brief Use a \ref transform on a bitmap, the rotation will be first
    ///        executet and use the ConvertFn on every value
    template <typename T, typename ConvertFn >
    inline bitmap<decltype(std::declval<ConvertFn>()(std::declval<T>()))> bitmap_transform(
        bitmap<T> const& image,
        ::bmp::transform transform,
        ConvertFn&& converter = &no_convert<T>) {
        using result_type = decltype(std::declval<ConvertFn>()(std::declval<T>()));
        using size_type = typename bitmap<result_type>::size_type;

        using enum ::bmp::transform;
        bool const rotation = is_any_set(transform, rigth_rotate);
        bool const mirror_h = is_any_set(transform, mirror_horizontal);
        bool const mirror_v = is_any_set(transform, mirror_vectical);

        bitmap<result_type> result(rotation ? size_type(image.h(), image.w()) : image.size());

        for(std::size_t y = 0; y < result.h(); ++y) {
            for(std::size_t x = 0; x < result.w(); ++x) {
                typename bitmap<result_type>::point_type target(x, y);

                if(rotation) {
                    target.set(target.y(), image.h() - target.x() - 1);
                }

                if(rotation ? mirror_v : mirror_h) {
                    target.x() = image.w() - target.x() - 1;
                }

                if(rotation ? mirror_h : mirror_v) {
                    target.y() = image.h() - target.y() - 1;
                }

                result(x, y) = converter(image(target));
            }
        }

        return result;
    }


}
