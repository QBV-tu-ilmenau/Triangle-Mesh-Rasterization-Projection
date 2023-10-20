#pragma once

#include <cstdint>
#include <utility>


namespace bmp::pixel {


    template <typename T>
    struct basic_ga {
        using value_type = T;

        static constexpr std::size_t channel_count = 2;

        template <typename U>
        using with_channel_type = basic_ga<U>;

        static basic_ga<T>
            fill_channels(T const& v) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            return {v, v};
        }

        template <typename U>
        explicit constexpr operator basic_ga<U>() noexcept(std::is_nothrow_convertible_v<T, U>) {
            return {static_cast<U>(g), static_cast<U>(a)};
        }

        T g;
        T a;

        [[nodiscard]] bool operator==(basic_ga const&) const = default;
    };

    using ga8 = basic_ga<std::int8_t>;
    using ga16 = basic_ga<std::int16_t>;
    using ga32 = basic_ga<std::int32_t>;
    using ga64 = basic_ga<std::int64_t>;
    using ga8u = basic_ga<std::uint8_t>;
    using ga16u = basic_ga<std::uint16_t>;
    using ga32u = basic_ga<std::uint32_t>;
    using ga64u = basic_ga<std::uint64_t>;
    using ga32f = basic_ga<float>;
    using ga64f = basic_ga<double>;


    template <typename T>
    struct basic_rgb {
        using value_type = T;

        static constexpr std::size_t channel_count = 3;

        template <typename U>
        using with_channel_type = basic_rgb<U>;

        static basic_rgb<T>
            fill_channels(T const& v) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            return {v, v, v};
        }

        template <typename U>
        explicit constexpr operator basic_rgb<U>() noexcept(std::is_nothrow_convertible_v<T, U>) {
            return {static_cast<U>(r), static_cast<U>(g), static_cast<U>(b)};
        }

        T r;
        T g;
        T b;

        [[nodiscard]] bool operator==(basic_rgb const&) const = default;
    };

    using rgb8 = basic_rgb<std::int8_t>;
    using rgb16 = basic_rgb<std::int16_t>;
    using rgb32 = basic_rgb<std::int32_t>;
    using rgb64 = basic_rgb<std::int64_t>;
    using rgb8u = basic_rgb<std::uint8_t>;
    using rgb16u = basic_rgb<std::uint16_t>;
    using rgb32u = basic_rgb<std::uint32_t>;
    using rgb64u = basic_rgb<std::uint64_t>;
    using rgb32f = basic_rgb<float>;
    using rgb64f = basic_rgb<double>;


    template <typename T>
    struct basic_rgba {
        using value_type = T;

        static constexpr std::size_t channel_count = 4;

        template <typename U>
        using with_channel_type = basic_rgba<U>;

        static basic_rgba<T>
            fill_channels(T const& v) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            return {v, v, v, v};
        }

        template <typename U>
        explicit constexpr operator basic_rgba<U>() noexcept(std::is_nothrow_convertible_v<T, U>) {
            return {static_cast<U>(r), static_cast<U>(g), static_cast<U>(b), static_cast<U>(a)};
        }

        T r;
        T g;
        T b;
        T a;

        [[nodiscard]] bool operator==(basic_rgba const&) const = default;
    };

    using rgba8 = basic_rgba<std::int8_t>;
    using rgba16 = basic_rgba<std::int16_t>;
    using rgba32 = basic_rgba<std::int32_t>;
    using rgba64 = basic_rgba<std::int64_t>;
    using rgba8u = basic_rgba<std::uint8_t>;
    using rgba16u = basic_rgba<std::uint16_t>;
    using rgba32u = basic_rgba<std::uint32_t>;
    using rgba64u = basic_rgba<std::uint64_t>;
    using rgba32f = basic_rgba<float>;
    using rgba64f = basic_rgba<double>;


    template <typename T>
    struct is_pixel_type: std::false_type {};

    template <typename T>
    struct is_pixel_type<basic_ga<T>>: std::true_type {};

    template <typename T>
    struct is_pixel_type<basic_rgb<T>>: std::true_type {};

    template <typename T>
    struct is_pixel_type<basic_rgba<T>>: std::true_type {};

    template <typename T>
    constexpr bool is_pixel_type_v = is_pixel_type<T>::value;


    template <typename T, bool = is_pixel_type_v<T>>
    struct channel_type {
        using type = T;
    };

    template <typename T>
    struct channel_type<T, true> {
        using type = typename T::value_type;
    };

    template <typename T>
    using channel_type_t = typename channel_type<T>::type;


    template <typename T, typename U, bool = is_pixel_type_v<T>>
    struct with_channel_type {
        using type = U;
    };

    template <typename T, typename U>
    struct with_channel_type<T, U, true> {
        using type = typename T::template with_channel_type<U>;
    };

    template <typename T, typename U>
    using with_channel_type_t = typename with_channel_type<T, U>::type;


    template <typename T, bool = is_pixel_type_v<T>>
    struct channel_count: std::integral_constant<std::size_t, 1> {};

    template <typename T>
    struct channel_count<T, true>: std::integral_constant<std::size_t, T::channel_count> {};

    template <typename T>
    constexpr std::size_t channel_count_v = channel_count<T>::value;


    template <typename T, bool = is_pixel_type_v<T>>
    struct fill_channels_type {
        constexpr T operator()(T&& v) {
            return static_cast<T&&>(v);
        }
        constexpr T operator()(T const& v) {
            return v;
        }
    };

    template <typename T>
    struct fill_channels_type<T, true> {
        constexpr auto operator()(channel_type_t<T> const& v) noexcept(
            std::is_nothrow_copy_constructible_v<T>) {
            return T::fill_channels(v);
        }
    };

    template <typename T>
    fill_channels_type<T> fill_channels{};


}
