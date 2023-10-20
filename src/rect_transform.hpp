#pragma once

#include "bitmap.hpp"
#include "interpolate.hpp"
#include "masked_pixel.hpp"
#include "matrix3x3.hpp"
#include "rect.hpp"

#include <array>
#include <cmath>
#include <limits>


namespace bmp {


    template <typename T>
    constexpr matrix3x3<T> solve(std::array<point<T>, 4> const& points) {
        auto p1 = points[0];
        auto p2 = points[1];
        auto p3 = points[2];
        auto p4 = points[3];

        T m[3][4]
            = {{p1.x(), p2.x(), p3.x(), p4.x()}, {p1.y(), p2.y(), p3.y(), p4.y()}, {1, 1, 1, 1}};

        gaussian_elimination(m);

        return matrix3x3<T>{
            {{m[0][3] * p1.x(), m[1][3] * p2.x(), m[2][3] * p3.x()},
             {m[0][3] * p1.y(), m[1][3] * p2.y(), m[2][3] * p3.y()},
             {m[0][3], m[1][3], m[2][3]}}};
    }


    template <typename T>
    constexpr matrix3x3<T> operator*(matrix3x3<T> const& m1, matrix3x3<T> const& m2) {
        matrix3x3<T> m{};

        for(std::size_t y = 0; y < 3; ++y) {
            for(std::size_t x = 0; x < 3; ++x) {
                for(std::size_t i = 0; i < 3; ++i) {
                    m.d[y][x] += m1.d[y][i] * m2.d[i][x];
                }
            }
        }

        return m;
    }

    template <typename T>
    constexpr point<T> transform_point(matrix3x3<T> const& m, point<T> const& p) {
        T v[3]{p.x(), p.y(), 1};
        T r[3]{};

        for(std::size_t y = 0; y < 3; ++y) {
            for(std::size_t x = 0; x < 3; ++x) {
                r[y] += m.d[y][x] * v[x];
            }
        }

        return point<T>{r[0] / r[2], r[1] / r[2]};
    }


    template <typename T>
    constexpr matrix3x3<T> rect_transform_homography(
        std::array<point<T>, 4> const& from,
        std::array<point<T>, 4> const& to) {
        return solve(to) * invert(solve(from));
    }

    template <typename T>
    std::array<point<T>, 4> transform_image_contour(
        matrix3x3<T> const& homography,
        size<std::size_t> const& image_size) {
        return {
            transform_point(homography, point<T>{0, 0}),
            transform_point(homography, point<T>{static_cast<T>(image_size.w()), 0}),
            transform_point(homography, point<T>{0, static_cast<T>(image_size.h())}),
            transform_point(homography, static_cast<point<T>>(to_point(image_size)))};
    }

    template <typename T>
    rect<long, long, std::size_t, std::size_t> image_contour(std::array<point<T>, 4> const& c) {
        using std::max;
        using std::min;

        auto const top_left = point{
            static_cast<long>(std::floor(min(min(c[0].x(), c[1].x()), min(c[2].x(), c[3].x())))),
            static_cast<long>(std::floor(min(min(c[0].y(), c[1].y()), min(c[2].y(), c[3].y()))))};
        auto const bottom_right = point{
            static_cast<long>(std::floor(max(max(c[0].x(), c[1].x()), max(c[2].x(), c[3].x())))),
            static_cast<long>(std::floor(max(max(c[0].y(), c[1].y()), max(c[2].y(), c[3].y()))))};
        return static_cast<rect<long, long, std::size_t, std::size_t>>(
            rect{top_left, bottom_right});
    }

    template <typename TVT, typename VT, typename T>
    auto transform_bitmap(
        matrix3x3<T> const& homography,
        bitmap<VT> const& image,
        rect<long, long, std::size_t, std::size_t> const& target_contour) {
        constexpr auto has_quiet_NaN
            = std::numeric_limits<pixel::channel_type_t<TVT>>::has_quiet_NaN;
        using pixel_type = std::conditional_t<has_quiet_NaN, TVT, pixel::basic_masked_pixel<TVT>>;

        bitmap<pixel_type> result(target_contour.size());

        for(std::size_t y = 0; y < result.h(); ++y) {
            for(std::size_t x = 0; x < result.w(); ++x) {
                auto const source_point = transform_point(
                    homography,
                    point<T>{
                        static_cast<T>(x) + static_cast<T>(target_contour.x()),
                        static_cast<T>(y) + static_cast<T>(target_contour.y())});

                auto const source_x = std::floor(source_point.x());
                auto const source_y = std::floor(source_point.y());

                if(source_x < 0 || source_y < 0 ||
                    source_x + 1 >= image.template w_as<T>() || source_y + 1 >= image.template h_as<T>()) {
                    if constexpr(has_quiet_NaN) {
                        result(x, y) = pixel::fill_channels<TVT>(
                            std::numeric_limits<pixel::channel_type_t<TVT>>::quiet_NaN());
                    } else {
                        result(x, y).m = false;
                    }
                } else {
                    auto const ratio_x = source_point.x() - source_x;
                    auto const ratio_y = source_point.y() - source_y;
                    auto const sx = static_cast<std::size_t>(source_x);
                    auto const sy = static_cast<std::size_t>(source_y);
                    auto v = static_cast<TVT>(interpolate_2d(
                        ratio_x,
                        ratio_y,
                        image(sx, sy),
                        image(sx + 1, sy),
                        image(sx, sy + 1),
                        image(sx + 1, sy + 1)));
                    if constexpr(has_quiet_NaN) {
                        result(x, y) = v;
                    } else {
                        result(x, y).v = v;
                        result(x, y).m = true;
                    }
                }
            }
        }

        return result;
    }

    template <typename VT, typename T>
    auto transform_bitmap(
        matrix3x3<T> const& homography,
        bitmap<VT> const& image,
        rect<long, long, std::size_t, std::size_t> const& target_contour) {
        return transform_bitmap<VT, VT, T>(homography, image, target_contour);
    }


}
