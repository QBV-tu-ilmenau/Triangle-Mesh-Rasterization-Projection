#pragma once

#include "bitmap.hpp"
#include "interpolate.hpp"
#include "rect.hpp"

#include <cmath>
#include <type_traits>


namespace bmp::detail {


    template <typename T>
    void copy(
        bitmap<T>& target,
        bitmap<T> const& ref,
        rect<std::size_t> const& rect,
        point<std::size_t> const& target_start = {}) {
        for(std::size_t y = 0; y < rect.h(); ++y) {
            auto const ref_from = ref.begin() + (rect.y() + y) * ref.w() + rect.x();

            auto const target_from
                = target.begin() + (target_start.y() + y) * target.w() + target_start.x();

            std::copy(ref_from, ref_from + rect.w(), target_from);
        }
    }


    template <typename FXT, typename FYT, typename T>
    void interpolate_2d(
        bitmap<T>& target,
        bitmap<T> const& ref,
        rect<std::size_t> const& rect,
        point<FXT, FYT> const& ratio,
        point<std::size_t> const& target_start = {}) {
        for(std::size_t y = 0; y < rect.h(); ++y) {
            for(std::size_t x = 0; x < rect.w(); ++x) {
                auto const ax = rect.x() + x;
                auto const ay = rect.y() + y;
                target(target_start.x() + x, target_start.y() + y)
                    = static_cast<T>(bmp::interpolate_2d(
                        ratio.x(),
                        ratio.y(),
                        ref(ax, ay),
                        ref(ax + 1, ay),
                        ref(ax, ay + 1),
                        ref(ax + 1, ay + 1)));
            }
        }
    }


    template <typename FT, typename T>
    void x_interpolate(
        bitmap<T>& target,
        bitmap<T> const& ref,
        rect<std::size_t> const& rect,
        FT xratio,
        point<std::size_t> const& target_start = {}) {
        for(std::size_t y = 0; y < rect.h(); ++y) {
            for(std::size_t x = 0; x < rect.w(); ++x) {
                auto const ax = rect.x() + x;
                auto const ay = rect.y() + y;
                target(target_start.x() + x, target_start.y() + y)
                    = static_cast<T>(bmp::interpolate(xratio, ref(ax, ay), ref(ax + 1, ay)));
            }
        }
    }

    template <typename FT, typename T>
    void y_interpolate(
        bitmap<T>& target,
        bitmap<T> const& ref,
        rect<std::size_t> const& rect,
        FT y_ratio,
        point<std::size_t> const& target_start = {}) {
        for(std::size_t y = 0; y < rect.h(); ++y) {
            for(std::size_t x = 0; x < rect.w(); ++x) {
                auto const ax = rect.x() + x;
                auto const ay = rect.y() + y;
                target(target_start.x() + x, target_start.y() + y)
                    = static_cast<T>(bmp::interpolate(y_ratio, ref(ax, ay), ref(ax, ay + 1)));
            }
        }
    }


    template <typename XT, typename YT, typename WT, typename HT>
    std::string out_of_range_msg(
        size<std::size_t> const& bmp_size,
        point<XT, YT> const& tl,
        size<WT, HT> const& rect_size) {
        std::ostringstream os;
        os << "subbitmap: rect(point(x = " << tl.x();
        if(std::is_floating_point_v<XT>) {
            os << "[float -> max(x) = " << std::floor(tl.x()) << " + 1]";
        }
        os << ", y = " << tl.y();
        if(std::is_floating_point_v<YT>) {
            os << "[float -> max(y) = " << std::floor(tl.y()) << " + 1]";
        }
        os << "), size(w = " << rect_size.w() << ", h = " << rect_size.h()
           << ")) is outside the original bitmap(w = " << bmp_size.w() << ", h = " << bmp_size.h()
           << ")";
        return os.str();
    }

    template <typename XT, typename YT, typename WT, typename HT>
    std::string neg_size_msg(rect<XT, YT, WT, HT> const& rect) {
        std::ostringstream os;
        os << "subbitmap: rect(point(x = " << rect.x() << ", y = " << rect.y()
           << "), size(w = " << rect.w() << ", h = " << rect.h() << ")) has negetive size";
        return os.str();
    }

    template <typename XT, typename YT, typename WT, typename HT>
    void subbitmap_check_rect(rect<XT, YT, WT, HT> const& rect) {
        static_assert(
            std::is_arithmetic_v<XT> && std::is_arithmetic_v<YT>,
            "rect must have arithmetic x and y");

        static_assert(
            std::is_integral_v<WT> && std::is_integral_v<HT>,
            "rect must have integral w and h");

        if(rect.w() < 0 || rect.h() < 0) {
            throw std::logic_error(detail::neg_size_msg(rect));
        }
    }


    template <typename T>
    std::size_t to_size_t(T v) noexcept {
        if constexpr(std::is_integral_v<T>) {
            return static_cast<std::size_t>(v);
        } else {
            return static_cast<std::size_t>(std::floor(v));
        }
    }

    template <typename T>
    constexpr bool is_integral(T v) noexcept {
        if constexpr(std::is_integral_v<T>) {
            return true;
        } else {
            return v == std::floor(v);
        }
    }

    template <typename T, typename XT, typename YT>
    bitmap<T> subbitmap(
        bitmap<T> const& org,
        point<XT, YT> const& top_left,
        rect<std::size_t> const& int_rect) {
        if constexpr(std::is_integral_v<XT> && std::is_integral_v<YT>) {
            if(int_rect.x() + int_rect.w() > org.w() || int_rect.y() + int_rect.h() > org.h()) {
                throw std::out_of_range(out_of_range_msg(org.size(), top_left, int_rect.size()));
            }

            bitmap<T> result(int_rect.size());
            copy(result, org, int_rect);
            return result;
        } else if constexpr(std::is_integral_v<XT>) {
            if(int_rect.x() + int_rect.w() > org.w() || int_rect.y() + 1 + int_rect.h() > org.h()) {
                throw std::out_of_range(out_of_range_msg(org.size(), top_left, int_rect.size()));
            }

            auto const y_ratio = 1 - (top_left.y() - std::floor(top_left.y()));
            bitmap<T> result(int_rect.size());
            y_interpolate(result, org, int_rect, y_ratio);
            return result;
        } else if constexpr(std::is_integral_v<YT>) {
            if(int_rect.x() + 1 + int_rect.w() > org.w() || int_rect.y() + int_rect.h() > org.h()) {
                throw std::out_of_range(out_of_range_msg(org.size(), top_left, int_rect.size()));
            }

            auto const x_ratio = 1 - (top_left.x() - std::floor(top_left.x()));
            bitmap<T> result(int_rect.size());
            x_interpolate(result, org, int_rect, x_ratio);
            return result;
        } else {
            if(int_rect.x() + 1 + int_rect.w() > org.w()
               || int_rect.y() + 1 + int_rect.h() > org.h()) {
                throw std::out_of_range(out_of_range_msg(org.size(), top_left, int_rect.size()));
            }

            auto const ratio = point(
                top_left.x() - std::floor(top_left.x()),
                top_left.y() - std::floor(top_left.y()));
            bitmap<T> result(int_rect.size());
            interpolate_2d(result, org, int_rect, ratio);
            return result;
        }
    }


}


namespace bmp {


    /// \brief Return the pixels in rect as new bitmap, throw if out of range
    template <typename T, typename XT, typename YT, typename WT, typename HT>
    bitmap<T> subbitmap(bitmap<T> const& org, rect<XT, YT, WT, HT> const& rect) {
        detail::subbitmap_check_rect(rect);

        if(rect.x() < 0 || rect.y() < 0) {
            throw std::out_of_range(detail::out_of_range_msg(org.size(), rect.pos(), rect.size()));
        }

        auto const int_rect = ::bmp::rect(
            point(detail::to_size_t(rect.x()), detail::to_size_t(rect.y())),
            size(detail::to_size_t(rect.w()), detail::to_size_t(rect.h())));

        auto const is_x_int = detail::is_integral(rect.x());
        auto const is_y_int = detail::is_integral(rect.y());
        if(is_x_int && is_y_int) {
            return detail::subbitmap(org, int_rect.pos(), int_rect);
        } else if(is_x_int) {
            return detail::subbitmap(org, point(int_rect.x(), rect.y()), int_rect);
        } else if(is_y_int) {
            return detail::subbitmap(org, point(rect.x(), int_rect.y()), int_rect);
        } else {
            return detail::subbitmap(org, rect.pos(), int_rect);
        }
    }


}
