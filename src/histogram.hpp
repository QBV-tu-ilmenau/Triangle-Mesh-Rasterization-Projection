#pragma once

#include "bitmap.hpp"

#include <cmath>


namespace bmp::detail {


    template <typename T>
    struct histogram_counter {
        std::vector<std::size_t>& histogram;
        bitmap<T> const& image;
        T const min;
        T const max;

        template <typename IndexFn>
        void operator()(IndexFn const& calc_index) noexcept {
            auto const min_check_fn = [min = min](auto const& fn) noexcept {
                return [&fn, min](auto v) { return fn(std::max(v, min)); };
            };
            auto const max_check_fn = [max = max](auto const& fn) noexcept {
                return [&fn, max](auto v) { return fn(std::min(v, max)); };
            };
            auto const minmax_check_fn = [&min_check_fn, &max_check_fn](auto const& fn) noexcept {
                return max_check_fn(min_check_fn(fn));
            };

            bool need_min_check
                = std::is_floating_point_v<T> ? true : std::numeric_limits<T>::min() < min;
            bool need_max_check
                = std::is_floating_point_v<T> ? true : std::numeric_limits<T>::max() > max;

            auto const count = [this](auto const& calc_index) noexcept {
                for(auto v: image) {
                    if constexpr(std::is_floating_point_v<T>) {
                        if(std::isnan(v))
                            continue;
                    }

                    ++histogram[calc_index(v)];
                }
            };

            if(need_min_check && need_max_check) {
                count(minmax_check_fn(calc_index));
            } else if(need_max_check) {
                count(max_check_fn(calc_index));
            } else if(need_min_check) {
                count(min_check_fn(calc_index));
            } else {
                count(calc_index);
            }
        }
    };


    template <typename T, bool = std::is_integral_v<T>>
    struct make_diff_type {
        using type = T;
    };

    template <typename T>
    struct make_diff_type<T, true> {
        using type = std::make_unsigned_t<T>;
    };

    template <typename T>
    using make_diff_type_t = typename make_diff_type<T>::type;


}


namespace bmp {


    template <typename T>
    std::vector<std::size_t> histogram(
        bitmap<T> const& image,
        T const min,
        T const max,
        std::size_t const bin_count,
        bool const cumulative = false) {
        // for signed integer types diff may not fit in singed range but it
        // fits in ever in the corresponding unsigned range
        using diff_type = detail::make_diff_type_t<T>;

        auto const diff = static_cast<diff_type>(max - min);
        auto const max_index = bin_count - 1;

        std::vector<std::size_t> result(bin_count);
        detail::histogram_counter<T> calc{result, image, min, max};
        if(min == 0 && diff == max_index) {
            calc([](auto v) noexcept { return static_cast<std::size_t>(v); });
        } else {
            calc([min, max_index, diff](auto v) noexcept {
                auto const v0 = static_cast<diff_type>(v - min);
                return static_cast<std::size_t>(v0 * max_index / diff);
            });
        }

        if(cumulative) {
            std::size_t sum = 0;
            for(auto& v: result) {
                sum += v;
                v = sum;
            }
        }

        return result;
    }


}
