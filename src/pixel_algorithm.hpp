#pragma once

#include "pixel.hpp"

#include <algorithm>


namespace bmp::pixel {


    template <typename T>
    constexpr basic_ga<T> min(basic_ga<T> const& l, basic_ga<T> const& r) noexcept {
        using std::min;
        return {min(l.g, r.g), min(l.a, r.a)};
    }

    template <typename T>
    constexpr basic_ga<T> max(basic_ga<T> const& l, basic_ga<T> const& r) noexcept {
        using std::max;
        return {max(l.g, r.g), max(l.a, r.a)};
    }

    template <typename T>
    constexpr basic_rgb<T> min(basic_rgb<T> const& l, basic_rgb<T> const& r) noexcept {
        using std::min;
        return {min(l.r, r.r), min(l.g, r.g), min(l.b, r.b)};
    }

    template <typename T>
    constexpr basic_rgb<T> max(basic_rgb<T> const& l, basic_rgb<T> const& r) noexcept {
        using std::max;
        return {max(l.r, r.r), max(l.g, r.g), max(l.b, r.b)};
    }

    template <typename T>
    constexpr basic_rgba<T> min(basic_rgba<T> const& l, basic_rgba<T> const& r) noexcept {
        using std::min;
        return {min(l.r, r.r), max(l.g, r.g), max(l.b, r.b), max(l.a, r.a)};
    }

    template <typename T>
    constexpr basic_rgba<T> max(basic_rgba<T> const& l, basic_rgba<T> const& r) noexcept {
        using std::max;
        return {max(l.r, r.r), max(l.g, r.g), max(l.b, r.b), max(l.a, r.a)};
    }

    template <typename T, typename UnaryOperation>
    constexpr bool any(T const& v, UnaryOperation const& unary_op) {
        return unary_op(v);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool any(basic_ga<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.g) || unary_op(v.a);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool any(basic_rgb<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.r) || unary_op(v.g) || unary_op(v.b);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool any(basic_rgba<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.r) || unary_op(v.g) || unary_op(v.b) || unary_op(v.a);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool all(T const& v, UnaryOperation const& unary_op) {
        return unary_op(v);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool all(basic_ga<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.g) && unary_op(v.a);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool all(basic_rgb<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.r) && unary_op(v.g) && unary_op(v.b);
    }

    template <typename T, typename UnaryOperation>
    constexpr bool all(basic_rgba<T> const& v, UnaryOperation const& unary_op) {
        return unary_op(v.r) && unary_op(v.g) && unary_op(v.b) && unary_op(v.a);
    }


}
