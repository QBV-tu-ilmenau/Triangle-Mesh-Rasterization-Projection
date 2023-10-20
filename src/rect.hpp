#pragma once

#include "point.hpp"
#include "size.hpp"

#include <algorithm>
#include <type_traits>


namespace bmp::detail {


    template <typename XT, typename YT, typename WT, typename HT>
    struct rect_common_base {
        [[nodiscard]] constexpr bool operator==(rect_common_base const&) const noexcept = default;
    };

    template <typename T>
    struct rect_common_base<T, T, T, T> {
        /// \brief Type of the x, y, width and height
        using value_type = T;

        [[nodiscard]] constexpr bool operator==(rect_common_base const&) const noexcept = default;
    };


    template <typename XT, typename YT, typename WT, typename HT>
    struct rect_base: rect_common_base<XT, YT, WT, HT> {
    public:
        /// \brief Type of the x position
        using x_type = XT;

        /// \brief Type of the y position
        using y_type = YT;

        /// \brief Type of the width
        using w_type = WT;

        /// \brief Type of the height
        using h_type = HT;

        /// \brief Type of the rect top left position
        using pos_type = ::bmp::point<XT, YT>;

        /// \brief Type of the rect size
        using size_type = ::bmp::size<WT, HT>;


        /// \brief Constructs a blank rect
        constexpr rect_base() noexcept = default;

        /// \brief Copy constructor
        constexpr rect_base(rect_base const&) noexcept = default;

        /// \brief Move constuctor
        constexpr rect_base(rect_base&&) noexcept = default;

        /// \brief Constructs on position (top_left.x, top_left.y), with size (size.w, size.h)
        constexpr rect_base(pos_type const& top_left, size_type const& size) noexcept
            : top_left_(top_left)
            , size_(size) {}

        /// \brief Constructs on position (0, 0), with size (size.w, size.h)
        constexpr rect_base(size_type const& size) noexcept
            : size_(size) {}

        /// \brief Constructs on position (0, 0), with size (w, h)
        constexpr rect_base(w_type const& w, h_type const& h) noexcept
            : size_(w, h) {}

        /// \brief Constructs on position (x, y), with size (w, h)
        constexpr rect_base(x_type const& x, y_type const& y, w_type const& w, h_type const& h) noexcept
            : top_left_(x, y)
            , size_(w, h) {}


        /// \brief Copy assignment
        constexpr rect_base& operator=(rect_base const&) noexcept = default;

        /// \brief Move assignment
        constexpr rect_base& operator=(rect_base&&) noexcept = default;


        /// \brief Get true, if width or height is zero
        [[nodiscard]] constexpr bool is_empty() const noexcept {
            return w() == w_type() && h() == h_type();
        }

        /// \brief Get true, if width and height are positv
        [[nodiscard]] constexpr bool is_positive() const noexcept {
            return size_.is_positive();
        }


        /// \brief Get the top left point
        [[nodiscard]] constexpr pos_type pos() const noexcept {
            return top_left_;
        }

        /// \brief Get the size
        [[nodiscard]] constexpr size_type size() const noexcept {
            return size_;
        }

        /// \brief Get the x position
        [[nodiscard]] constexpr x_type x() const noexcept {
            return top_left_.x();
        }

        /// \brief Get the y position
        [[nodiscard]] constexpr y_type y() const noexcept {
            return top_left_.y();
        }

        /// \brief Get the width
        [[nodiscard]] constexpr w_type w() const noexcept {
            return size_.w();
        }

        /// \brief Get the height
        [[nodiscard]] constexpr h_type h() const noexcept {
            return size_.h();
        }


        /// \brief Get abs(w) * abs(h)
        [[nodiscard]] constexpr auto area() const noexcept {
            return size_.area();
        }

        /// \brief Get abs(w) * abs(h)
        template <typename CommonType>
        [[nodiscard]] constexpr CommonType area_as() const noexcept {
            return size_.template area_as<CommonType>();
        }


        /// \brief Set x, y, width and height
        constexpr void set(pos_type const& pos, size_type const& size) noexcept {
            set_pos(pos);
            set_size(size);
        }

        /// \brief Set x, y, width and height
        constexpr void set(x_type const& x, y_type const& y, w_type const& w, h_type const& h) noexcept {
            set_pos(x, y);
            set_size(w, h);
        }

        /// \brief Set x and y
        constexpr void set_pos(pos_type const& pos) noexcept {
            top_left_ = pos;
        }

        /// \brief Set x and y
        constexpr void set_pos(x_type const& x, y_type const& y) noexcept {
            set_pos({x, y});
        }

        /// \brief Set width and height
        constexpr void set_size(size_type const& size) noexcept {
            size_ = size;
        }

        /// \brief Set width and height
        constexpr void set_size(w_type const& w, h_type const& h) noexcept {
            set_size({w, h});
        }

        /// \brief Set the x position
        constexpr void set_x(x_type const& x) noexcept {
            top_left_.set_x(x);
        }

        /// \brief Set the y position
        constexpr void set_y(y_type const& y) noexcept {
            top_left_.set_y(y);
        }

        /// \brief Set the width
        constexpr void set_w(w_type const& w) noexcept {
            size_.set_w(w);
        }

        /// \brief Set the height
        constexpr void set_h(h_type const& h) noexcept {
            size_.set_h(h);
        }


        [[nodiscard]] constexpr bool operator==(rect_base const&) const noexcept = default;


    private:
        /// \brief The point in the top left corner
        pos_type top_left_;

        /// \brief The point in the bottom right corner
        size_type size_;
    };


    template <typename XT, typename YT, typename WT, typename HT>
    struct rect_hv_base: rect_base<XT, YT, WT, HT> {
        using rect_base<XT, YT, WT, HT>::rect_base;

        [[nodiscard]] constexpr bool operator==(rect_hv_base const&) const noexcept = default;
    };


}


namespace bmp {


    /// \brief A class for manipulating rectangles
    template <typename XT, typename YT = XT, typename WT = XT, typename HT = YT>
    struct rect: detail::rect_hv_base<XT, YT, WT, HT> {
        using detail::rect_hv_base<XT, YT, WT, HT>::rect_hv_base;

        /// \brief Enable static casts
        template <typename X2T, typename Y2T = X2T, typename W2T = X2T, typename H2T = Y2T>
        [[nodiscard]] explicit constexpr operator rect<X2T, Y2T, W2T, H2T>() const noexcept {
            return {
                static_cast<point<X2T, Y2T>>(detail::rect_hv_base<XT, YT, WT, HT>::pos()),
                static_cast<size<W2T, H2T>>(detail::rect_hv_base<XT, YT, WT, HT>::size())};
        }

        [[nodiscard]] constexpr bool operator==(rect const&) const noexcept = default;
    };


    template <typename XT, typename YT, typename WT, typename HT>
    rect(XT, YT, WT, HT) -> rect<XT, YT, WT, HT>;

    template <typename XT, typename YT, typename WT, typename HT>
    rect(point<XT, YT>, size<WT, HT>) -> rect<XT, YT, WT, HT>;

    template <typename WT, typename HT>
    rect(WT, HT) -> rect<WT, HT, WT, HT>;

    template <typename WT, typename HT>
    rect(size<WT, HT>) -> rect<WT, HT, WT, HT>;

    template <typename XT, typename YT>
    rect(point<XT, YT>) -> rect<XT, YT, XT, YT>;

    template <typename XT, typename YT>
    rect(point<XT, YT>, point<XT, YT>) -> rect<XT, YT, XT, YT>;


    /// \brief Get true, if point is in rect
    template <typename XT, typename YT>
    [[nodiscard]] constexpr bool
        contains(rect<XT, YT, XT, YT> const& rect, point<XT, YT> const& point) noexcept {
        return point.x() >= rect.l() && point.x() <= rect.r() && point.y() >= rect.t()
            && point.y() <= rect.b();
    }

    /// \brief Get true, if test is in ref
    template <typename XT, typename YT>
    [[nodiscard]] constexpr bool
        contains(rect<XT, YT, XT, YT> const& ref, rect<XT, YT, XT, YT> const& test) noexcept {
        return test.l() >= ref.l() && test.r() <= ref.r() && test.t() >= ref.t()
            && test.b() <= ref.b();
    }


    /// \brief Get a rect that contains both rects
    template <typename XT, typename YT>
    [[nodiscard]] constexpr rect<XT, YT, XT, YT>
        join(rect<XT, YT, XT, YT> const& l, rect<XT, YT, XT, YT> const& r) noexcept {
        using std::max;
        using std::min;
        return {
            ::bmp::point(min(l.l(), r.l()), min(l.t(), r.t())),
            ::bmp::point(max(l.r(), r.r()), max(l.b(), r.b()))};
    }


}


namespace bmp::detail {


    template <typename XT, typename YT>
    struct rect_hv_base<XT, YT, XT, YT>: rect_base<XT, YT, XT, YT> {
        using typename rect_base<XT, YT, XT, YT>::pos_type;
        using typename rect_base<XT, YT, XT, YT>::size_type;

        /// \brief Type of the x and width
        using horizontal_type = XT;

        /// \brief Type of the y and height
        using vertical_type = YT;

        /// \brief true if x dimensions are integer types, false otherwise
        static constexpr bool is_x_half_open = std::numeric_limits<horizontal_type>::is_integer;

        /// \brief true if y dimensions are integer types, false otherwise
        static constexpr bool is_y_half_open = std::numeric_limits<vertical_type>::is_integer;


        using rect_base<XT, YT, XT, YT>::rect_base;


        /// \brief Constructs a rect on position (0, 0), with size
        ///        bottom_right.x + 1 as width and rb.y + 1 as height
        constexpr rect_hv_base(pos_type const& rb) noexcept
            : rect_base<XT, YT, XT, YT>(to_size(
                pos_type(
                    static_cast<horizontal_type>(is_x_half_open),
                    static_cast<vertical_type>(is_y_half_open))
                + rb)) {}

        /// \brief Constructs a rect on position (top_left.x, top_left.y), with
        ///        size bottom_right.x + 1 - top_left.x as width and
        ///        bottom_right.y + 1 - top_left.x as height
        constexpr rect_hv_base(pos_type const& lt, pos_type const& rb) noexcept
            : rect_base<XT, YT, XT, YT>(
                lt,
                to_size(
                    pos_type(
                        static_cast<horizontal_type>(is_x_half_open),
                        static_cast<vertical_type>(is_y_half_open))
                    + rb - lt)) {}


        using rect_base<XT, YT, XT, YT>::x;
        using rect_base<XT, YT, XT, YT>::y;
        using rect_base<XT, YT, XT, YT>::w;
        using rect_base<XT, YT, XT, YT>::h;
        using rect_base<XT, YT, XT, YT>::pos;
        using rect_base<XT, YT, XT, YT>::size;

        using rect_base<XT, YT, XT, YT>::set_x;
        using rect_base<XT, YT, XT, YT>::set_y;
        using rect_base<XT, YT, XT, YT>::set_w;
        using rect_base<XT, YT, XT, YT>::set_h;
        using rect_base<XT, YT, XT, YT>::set_pos;
        using rect_base<XT, YT, XT, YT>::set_size;


        /// \brief Get the top point
        [[nodiscard]] constexpr vertical_type t() const noexcept {
            return y();
        }

        /// \brief Get the bottom point
        [[nodiscard]] constexpr vertical_type b() const noexcept {
            return h() + y() - static_cast<vertical_type>(is_y_half_open);
        }

        /// \brief Get the left point
        [[nodiscard]] constexpr horizontal_type l() const noexcept {
            return x();
        }

        /// \brief Get the right point
        [[nodiscard]] constexpr horizontal_type r() const noexcept {
            return w() + x() - static_cast<horizontal_type>(is_x_half_open);
        }


        /// \brief Get the top left corner
        [[nodiscard]] constexpr pos_type lt() const noexcept {
            return pos();
        }

        /// \brief Get the top right corner
        [[nodiscard]] constexpr pos_type rt() const noexcept {
            return {r(), t()};
        }

        /// \brief Get the bottom left corner
        [[nodiscard]] constexpr pos_type lb() const noexcept {
            return {l(), b()};
        }

        /// \brief Get the bottom right corner
        [[nodiscard]] constexpr pos_type rb() const noexcept {
            return {r(), b()};
        }


        /// \brief Set the top point
        constexpr void set_t(vertical_type const& t) noexcept {
            set_h(h() + y() - t);
            set_y(t);
        }

        /// \brief Set the bottom point
        constexpr void set_b(vertical_type const& b) noexcept {
            set_h(static_cast<vertical_type>(is_y_half_open) + b - y());
        }

        /// \brief Set the left point
        constexpr void set_l(horizontal_type const& l) noexcept {
            set_w(w() + x() - l);
            set_x(l);
        }

        /// \brief Set the right point
        constexpr void set_r(horizontal_type const& r) noexcept {
            set_w(static_cast<horizontal_type>(is_x_half_open) + r - x());
        }


        /// \brief Set the top left corner
        constexpr void set_lt(pos_type const& lt) noexcept {
            set_l(lt.x());
            set_t(lt.y());
        }

        /// \brief Set the top right corner
        constexpr void set_rt(pos_type const& rt) noexcept {
            set_r(rt.x());
            set_t(rt.y());
        }

        /// \brief Set the bottom left corner
        constexpr void set_lb(pos_type const& lb) noexcept {
            set_l(lb.x());
            set_b(lb.y());
        }

        /// \brief Set the bottom right corner
        constexpr void set_rb(pos_type const& rb) noexcept {
            set_r(rb.x());
            set_b(rb.y());
        }


        [[nodiscard]] constexpr bool operator==(rect_hv_base const&) const noexcept = default;
    };


}
