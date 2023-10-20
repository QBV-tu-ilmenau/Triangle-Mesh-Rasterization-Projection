#pragma once

#include "point.hpp"
#include "size.hpp"

#include <numeric>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>


namespace bmp {


    /// \brief A bitmap for data manipulation
    /// \tparam T Type of the data that administrates the bitmap
    template <typename T>
    class bitmap {
    public:
        /// \brief Type of the data that administrates the bitmap
        using value_type = T;

        /// \brief Type of points in the bitmap
        using point_type = ::bmp::point<std::size_t>;

        /// \brief Type of bitmap size
        using size_type = ::bmp::size<std::size_t>;

        /// \brief Type of bitmap size as signed
        using ssize_type = ::bmp::size<std::ptrdiff_t>;

        /// \brief Type of a iterator for data
        using iterator = typename std::vector<value_type>::iterator;

        /// \brief Type of a iterator for const data
        using const_iterator = typename std::vector<value_type>::const_iterator;

        /// \brief Type of a reverse iterator for data
        using reverse_iterator = typename std::vector<value_type>::reverse_iterator;

        /// \brief Type of a reverse iterator for const data
        using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

        /// \brief Type of a reference to data
        using reference = typename std::vector<value_type>::reference;

        /// \brief Type of a const reference to data
        using const_reference = typename std::vector<value_type>::const_reference;


        /// \brief Constructs a blank bitmap
        bitmap() = default;

        /// \brief Constructs a bitmap by copying another one
        bitmap(bitmap const&) = default;

        /// \brief Constructs a bitmap by moving the data of another one
        bitmap(bitmap&&) = default;

        /// \brief Constructs a bitmap with size, initialize all values with value
        bitmap(size_type const& size, value_type const& value = value_type())
            : size_(size)
            , data_(point_count(), value) {}

        /// \brief Constructs a bitmap with size, initialize values in column order by range
        /// \throw std::out_of_range
        template <typename InputIterator>
        bitmap(size_type const& size, InputIterator first, InputIterator last)
            : size_(size)
            , data_(first, last) {
            if(data_.size() != point_count()) {
                throw std::logic_error(
                    "bitmap constructor size (" + std::to_string(size_.w()) + "x"
                    + std::to_string(size_.h()) + ") and iterator range ("
                    + std::to_string(data_.size()) + ") are incompatible");
            }
        }

        /// \brief Constructs a bitmap with size w and h, initialize all values with value
        bitmap(std::size_t w, std::size_t h, value_type const& value = value_type())
            : size_(w, h)
            , data_(point_count(), value) {}

        /// \brief Constructs a bitmap by a two dimensional array
        template <std::size_t W, std::size_t H>
        bitmap(T const(&array)[H][W])
            : size_(W, H)
            , data_(
                std::begin(reinterpret_cast<T const(&)[W*H]>(array)),
                std::end(reinterpret_cast<T const(&)[W*H]>(array)))
            {}


        /// \brief Standard destructor
        ~bitmap() = default;


        /// \brief Copy assignment
        bitmap& operator=(bitmap const& bitmap) = default;

        /// \brief Move assignment
        bitmap& operator=(bitmap&& bitmap) = default;


        /// \brief Get a iterator on begin of the data
        iterator begin() {
            return data_.begin();
        }

        /// \brief Get a const_iterator on begin of the data
        const_iterator begin() const {
            return cbegin();
        }

        /// \brief Get a const_iterator on begin of the data
        const_iterator cbegin() const {
            return data_.cbegin();
        }

        /// \brief Get a iterator behind the data
        iterator end() {
            return data_.end();
        }

        /// \brief Get a const_iterator behind the data
        const_iterator end() const {
            return cend();
        }

        /// \brief Get a const_iterator behind the data
        const_iterator cend() const {
            return data_.cend();
        }


        /// \brief Get a reverse_iterator on end of the data
        reverse_iterator rbegin() {
            return data_.rbegin();
        }

        /// \brief Get a const_reverse_iterator on end of the data
        const_reverse_iterator rbegin() const {
            return crbegin();
        }

        /// \brief Get a const_reverse_iterator on end of the data
        const_reverse_iterator crbegin() const {
            return data_.crbegin();
        }

        /// \brief Get a reverse_iterator before the data
        reverse_iterator rend() {
            return data_.rend();
        }

        /// \brief Get a const_reverse_iterator before the data
        const_reverse_iterator rend() const {
            return crend();
        }

        /// \brief Get a const_reverse_iterator before the data
        const_reverse_iterator crend() const {
            return data_.crend();
        }


        /// \brief Resize the data field
        /// \attention All pointers and iterators to the data become invalid
        void resize(std::size_t w, std::size_t h, value_type const& value = value_type()) {
            resize(size_type(w, h), value);
        }

        /// \brief Resize the data field
        /// \attention All pointers and iterators to the data become invalid
        void resize(size_type const& size, value_type const& value = value_type()) {
            data_.resize(size.area(), value);
            size_ = size;
        }

        /// \brief Resize to zero
        void clear()noexcept {
            data_.clear();
            size_ = {0, 0};
        }


        /// \brief Get the width
        std::size_t w() const {
            return size_.w();
        }

        /// \brief Get the height
        std::size_t h() const {
            return size_.h();
        }

        /// \brief Get the size
        size_type const size() const {
            return size_;
        }

        /// \brief Get the number of points in the bitmap
        std::size_t point_count() const {
            return size_.area();
        }


        /// \brief Get the width as signed
        std::ptrdiff_t sw() const {
            return static_cast<std::ptrdiff_t>(w());
        }

        /// \brief Get the height as signed
        std::ptrdiff_t sh() const {
            return static_cast<std::ptrdiff_t>(h());
        }

        /// \brief Get the size as signed
        ssize_type const ssize() const {
            return ssize_type(sw(), sh());
        }

        /// \brief Get the number of points in the bitmap as signed
        std::ptrdiff_t spoint_count() const {
            return static_cast<std::ptrdiff_t>(point_count());
        }


        /// \brief Get the width with requested type
        template <typename U>
        U w_as() const {
            auto const result = static_cast<U>(w());
            if(w() != static_cast<std::size_t>(result)) {
                throw std::runtime_error("value of width changed during conversion to the requested type");
            }
            return result;
        }

        /// \brief Get the height with requested type
        template <typename U>
        U h_as() const {
            auto const result = static_cast<U>(h());
            if(h() != static_cast<std::size_t>(result)) {
                throw std::runtime_error("value of height changed during conversion to the requested type");
            }
            return result;
        }

        /// \brief Get the size with requested type
        template <typename U>
        ::bmp::size<U> const size_as() const {
            return {w_as<U>(), h_as<U>()};
        }

        /// \brief Get the number of points in the bitmap with requested type
        template <typename U>
        U point_count_as() const {
            auto const count = point_count();
            auto const result = static_cast<U>(count);
            if(count != static_cast<std::size_t>(result)) {
                throw std::runtime_error("value of point count changed during conversion to the requested type");
            }
            return result;
        }


        /// \brief Get a pointer to data for direct manipulation
        value_type* data() {
            return data_.data();
        }

        /// \brief Get a pointer to constant data for direct read
        value_type const* data() const {
            return data_.data();
        }


        /// \brief Get a reference to the value by local coordinates
        /// \throw std::out_of_range in debug build
        reference operator()(std::size_t x, std::size_t y) {
            return operator()(point_type(x, y));
        }

        /// \brief Get a reference to the value by local coordinates
        /// \throw std::out_of_range in debug build
        reference operator()(point_type const& point) {
            throw_if_out_of_range(point);
            return data_[data_pos(point)];
        }

        /// \brief Get the value by local coordinates
        /// \throw std::out_of_range in debug build
        const_reference operator()(std::size_t x, std::size_t y) const {
            return operator()(point_type(x, y));
        }

        /// \brief Get the value by local coordinates
        /// \throw std::out_of_range in debug build
        const_reference operator()(point_type const& point) const {
            throw_if_out_of_range(point);
            return data_[data_pos(point)];
        }


        /// \brief Converts a lokal point in a index for direct data access
        /// \attention This function performs no range protection
        std::size_t data_pos(std::size_t x, std::size_t y) const {
            return data_pos(point_type(x, y));
        }

        /// \brief Converts a lokal point in a index for direct data access
        /// \attention This function performs no range protection
        std::size_t data_pos(point_type const& point) const {
            return point.y() * w() + point.x();
        }

        /// \brief true if image is empty, false otherwise
        bool empty() const {
            return data_.empty();
        }

        [[nodiscard]] bool operator==(bitmap const&) const = default;


    protected:
        /// \brief The rectangle for global position and size
        size_type size_;

        /// \brief The data field
        std::vector<value_type> data_;


        /// \brief Throws an exception, if the point is out of range
        void throw_if_out_of_range(point_type const& point) const {
#ifdef DEBUG
            if(!is_point_in_bitmap(*this, point)) {
                std::ostringstream os;
                os << "bitmap: point(x = " << point.x() << ", y = " << point.y()
                   << ") is outside the bitmap (w = " << w() << ", h = " << h() << ")";
                throw std::out_of_range(os.str());
            }
#else
            (void)point;
#endif
        }
    };


    template <typename T>
    bool is_point_in_bitmap(bitmap<T> const& image, typename bitmap<T>::point_type const& point) {
        if(point.x() < 0 || point.x() >= image.w() || point.y() < 0 || point.y() >= image.h()) {
            return false;
        }
        return true;
    }


    /// \brief std::vector of bitmaps
    template <typename T>
    using bitmap_vector = std::vector<bitmap<T>>;


}
