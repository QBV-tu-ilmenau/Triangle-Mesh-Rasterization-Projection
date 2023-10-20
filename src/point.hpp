#pragma once

#include <utility>


namespace bmp::detail {


    template <typename XT, typename YT>
    struct point_base {
        [[nodiscard]] constexpr bool operator==(point_base const&) const noexcept = default;
    };

    template <typename T>
    struct point_base<T, T> {
        /// \brief Type of the positions
        using value_type = T;

        [[nodiscard]] constexpr bool operator==(point_base const&) const noexcept = default;
    };


}


namespace bmp {


    /// \brief A class for representing points
    /// \tparam XT Type of the x position data
    /// \tparam YT Type of the y position data
    template <typename XT, typename YT = XT>
    class point: public detail::point_base<XT, YT> {
    public:
        /// \brief Type of the x positions
        using x_type = XT;

        /// \brief Type of the y positions
        using y_type = YT;


        /// \brief Constructs a point by (0, 0)
        constexpr point()
            : x_()
            , y_() {}

        /// \brief Copy constructor
        constexpr point(point const&) noexcept = default;

        /// \brief Move constructor
        constexpr point(point&&) noexcept = default;

        /// \brief Constructs a point by (x, y)
        constexpr point(x_type const& x, y_type const& y) noexcept
            : x_(x)
            , y_(y) {}


        /// \brief Enable static casts
        template <typename X2T, typename Y2T>
        [[nodiscard]] explicit constexpr operator point<X2T, Y2T>() const noexcept {
            return {static_cast<X2T>(x_), static_cast<Y2T>(y_)};
        }


        /// \brief Copy assignment
        constexpr point& operator=(point const&) noexcept = default;

        /// \brief Move assignment
        constexpr point& operator=(point&&) noexcept = default;


        /// \brief Set x
        constexpr void set_x(x_type const& x) noexcept {
            x_ = x;
        }

        /// \brief Set y
        constexpr void set_y(y_type const& y) noexcept {
            y_ = y;
        }


        /// \brief Get x
        [[nodiscard]] constexpr x_type x() const noexcept {
            return x_;
        }

        /// \brief Get y
        [[nodiscard]] constexpr y_type y() const noexcept {
            return y_;
        }


        /// \brief Set x and y
        constexpr void set(x_type const& x, y_type const& y) noexcept {
            x_ = x;
            y_ = y;
        }


        /// \brief Get true, if x and y are positiv
        [[nodiscard]] constexpr bool is_positive() const noexcept {
            return x() >= x_type() && y() >= y_type();
        }


        [[nodiscard]] constexpr bool operator==(point const&) const noexcept = default;


        constexpr point& operator+=(point const& v) noexcept {
            x_ += v.x();
            y_ += v.y();
            return *this;
        }

        constexpr point& operator-=(point const& v) noexcept {
            x_ -= v.x();
            y_ -= v.y();
            return *this;
        }

        constexpr point& operator*=(point const& v) noexcept {
            x_ *= v.x();
            y_ *= v.y();
            return *this;
        }

        constexpr point& operator/=(point const& v) noexcept {
            x_ /= v.x();
            y_ /= v.y();
            return *this;
        }

        constexpr point& operator%=(point const& v) noexcept {
            x_ %= v.x();
            y_ %= v.y();
            return *this;
        }

    private:
        x_type x_;
        y_type y_;
    };


    template <typename XT, typename YT>
    [[nodiscard]] constexpr point<XT, YT> operator+(point<XT, YT> a, point<XT, YT> const& b) noexcept {
        return a += b;
    }

    template <typename XT, typename YT>
    [[nodiscard]] constexpr point<XT, YT> operator-(point<XT, YT> a, point<XT, YT> const& b) noexcept {
        return a -= b;
    }

    template <typename XT, typename YT>
    [[nodiscard]] constexpr point<XT, YT> operator*(point<XT, YT> a, point<XT, YT> const& b) noexcept {
        return a *= b;
    }

    template <typename XT, typename YT>
    [[nodiscard]] constexpr point<XT, YT> operator/(point<XT, YT> a, point<XT, YT> const& b) noexcept {
        return a /= b;
    }

    template <typename XT, typename YT>
    [[nodiscard]] constexpr point<XT, YT> operator%(point<XT, YT> a, point<XT, YT> const& b) noexcept {
        return a %= b;
    }


    template <typename WT, typename HT>
    class size;

    template <typename XT, typename YT>
    [[nodiscard]] size<XT, YT> to_size(point<XT, YT> const& p) noexcept {
        return {p.x(), p.y()};
    }


}
