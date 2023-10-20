#pragma once

#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <utility>


namespace bmp::detail {


    template <typename WT, typename HT>
    struct size_base {
        [[nodiscard]] constexpr bool operator==(size_base const&) const noexcept = default;
    };

    template <typename T>
    struct size_base<T, T> {
        /// \brief Type of width and height
        using value_type = T;

        [[nodiscard]] constexpr bool operator==(size_base const&) const noexcept = default;
    };


}


namespace bmp {


    /// \brief A class for manipulating sizes
    template <typename WT, typename HT = WT>
    class size: public detail::size_base<WT, HT> {
    public:
        /// \brief Type of the width
        using w_type = WT;

        /// \brief Type of the height
        using h_type = HT;


        /// \brief Constructs a size with width 0 and height 0
        constexpr size() noexcept
            : w_()
            , h_() {}

        /// \brief Copy constructor
        constexpr size(size const&) noexcept = default;

        /// \brief Move constructor
        constexpr size(size&&) noexcept = default;

        /// \brief Constructs a size with width and height
        constexpr size(w_type const& w, h_type const& h) noexcept
            : w_(w)
            , h_(h) {}


        /// \brief Enable static casts
        template <typename W2T, typename H2T>
        explicit constexpr operator size<W2T, H2T>() const noexcept {
            return {static_cast<W2T>(w_), static_cast<H2T>(h_)};
        }


        /// \brief Copy assignment
        constexpr size& operator=(size const&) noexcept = default;

        /// \brief Move assignment
        constexpr size& operator=(size&&) noexcept = default;


        /// \brief Set width
        constexpr void set_w(w_type const& w) noexcept {
            w_ = w;
        }

        /// \brief Set height
        constexpr void set_h(h_type const& h) noexcept {
            h_ = h;
        }


        /// \brief Get width
        [[nodiscard]] constexpr w_type w() const noexcept {
            return w_;
        }

        /// \brief Get height
        [[nodiscard]] constexpr h_type h() const noexcept {
            return h_;
        }


        /// \brief Set width and height
        constexpr void set(w_type const& w, h_type const& h) noexcept {
            w_ = w;
            h_ = h;
        }


        /// \brief Get true, if width and height are positiv
        [[nodiscard]] constexpr bool is_positive() const noexcept {
            return w() >= w_type() && h() >= h_type();
        }


        /// \brief Get abs(w) * abs(h)
        [[nodiscard]] constexpr auto area() const noexcept {
            return area_as<std::common_type_t<w_type, h_type>>();
        }

        /// \brief Get abs(w) * abs(h)
        template <typename CommonType>
        [[nodiscard]] constexpr CommonType area_as() const noexcept {
            auto convert = [](auto const& v) {
                using type = std::remove_reference_t<decltype(v)>;
                if constexpr(std::is_signed_v<type>) {
                    using std::abs;
                    return static_cast<CommonType>(abs(v));
                } else {
                    return static_cast<CommonType>(v);
                }
            };
            return convert(w()) * convert(h());
        }

        [[nodiscard]] constexpr bool operator==(size const&) const = default;


        constexpr size& operator+=(size const& v) noexcept {
            w_ += v.w();
            h_ += v.h();
            return *this;
        }

        constexpr size& operator-=(size const& v) noexcept {
            w_ -= v.w();
            h_ -= v.h();
            return *this;
        }

        constexpr size& operator*=(size const& v) noexcept {
            w_ *= v.w();
            h_ *= v.h();
            return *this;
        }

        constexpr size& operator/=(size const& v) noexcept {
            w_ /= v.w();
            h_ /= v.h();
            return *this;
        }

        constexpr size& operator%=(size const& v) noexcept {
            w_ %= v.w();
            h_ %= v.h();
            return *this;
        }


    private:
        w_type w_;
        h_type h_;
    };


    template <typename WT, typename HT>
    [[nodiscard]] constexpr size<WT, HT> operator+(size<WT, HT> a, size<WT, HT> const& b) noexcept {
        return a += b;
    }

    template <typename WT, typename HT>
    [[nodiscard]] constexpr size<WT, HT> operator-(size<WT, HT> a, size<WT, HT> const& b) noexcept {
        return a -= b;
    }

    template <typename WT, typename HT>
    [[nodiscard]] constexpr size<WT, HT> operator*(size<WT, HT> a, size<WT, HT> const& b) noexcept {
        return a *= b;
    }

    template <typename WT, typename HT>
    [[nodiscard]] constexpr size<WT, HT> operator/(size<WT, HT> a, size<WT, HT> const& b) noexcept {
        return a /= b;
    }

    template <typename WT, typename HT>
    [[nodiscard]] constexpr size<WT, HT> operator%(size<WT, HT> a, size<WT, HT> const& b) noexcept {
        return a %= b;
    }


    template <typename WT, typename HT>
    class point;

    template <typename WT, typename HT>
    [[nodiscard]] point<WT, HT> to_point(size<WT, HT> const& s) noexcept {
        return {s.w(), s.h()};
    }

    /// \brief Get true, if point values are both smaller then size values
    template <typename XT, typename YT>
    [[nodiscard]] constexpr bool contains(size<XT, YT> const& size, point<XT, YT> const& point) noexcept {
        return point.x() < size.w() && point.y() < size.h();
    }


}
