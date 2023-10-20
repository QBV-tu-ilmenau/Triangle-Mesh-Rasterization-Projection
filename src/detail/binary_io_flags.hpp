#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>


namespace bmp::detail {


    template <typename T>
    concept endian_supported = std::integral<T> || std::same_as<T, float> || std::same_as<T, double>;

    template <endian_supported T>
    struct integer_type_t{
        using type = T;
    };

    template <>
    struct integer_type_t<float>{
        using type = std::uint32_t;
    };

    template <>
    struct integer_type_t<double>{
        using type = std::uint64_t;
    };

    template <endian_supported T>
    using integer_type = integer_type_t<T>::type;

    template <endian_supported T>
    constexpr integer_type<T> byteswap_to_integer(T const value) noexcept {
        if constexpr(std::integral<T>){
            return std::byteswap(value);
        }else{
            return std::byteswap(std::bit_cast<integer_type<T>>(value));
        }
    }

    template <endian_supported T>
    constexpr T byteswap_to(integer_type<T> const value) noexcept {
        if constexpr(std::integral<T>){
            return std::byteswap(value);
        }else{
            return std::bit_cast<T>(std::byteswap(value));
        }
    }

    template <std::integral T>
    constexpr T byteswap_on_little_endian(T const value) noexcept {
        if constexpr(std::endian::native == std::endian::little){
            return std::byteswap(value);
        }else{
            return value;
        }
    }


    constexpr auto big_endian_io_magic = byteswap_on_little_endian(std::uint32_t(0x62626621));

    enum class binary_type_flags : std::uint8_t {
        is_unsigned = 0x00,
        is_signed = 0x01,
        is_floating_point = 0x02,
        is_bool = 0x03
    };

    enum class binary_endian_flags : std::uint8_t { is_big_endian = 0x00, is_little_endian = 0x10 };

    // clang-format off
    template <typename T>
    constexpr std::uint8_t binary_io_flags_v
        = static_cast<std::uint8_t>(std::is_same_v<T, bool>
            ? binary_type_flags::is_bool
            : std::is_floating_point_v<T>
            ? binary_type_flags::is_floating_point
            : std::is_signed_v<T>
            ? binary_type_flags::is_signed
            : binary_type_flags::is_unsigned)
        | static_cast<std::uint8_t>(std::endian::big == std::endian::native
            ? binary_endian_flags::is_big_endian
            : binary_endian_flags::is_little_endian);
    // clang-format on


}
