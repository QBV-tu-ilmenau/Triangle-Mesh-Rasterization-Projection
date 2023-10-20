#pragma once

#include "bitmap.hpp"
#include "exception.hpp"

#include "detail/binary_io_flags.hpp"
#include "detail/valid_binary_format.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <string>


namespace bmp {


    /// \brief Write bitmap to std::ostream
    ///
    /// \throw binary_io_error
    template <typename T>
    void binary_write(bitmap<T> const& bitmap, std::ostream& os, std::endian endianness = std::endian::native) {
        static_assert(
            detail::is_valid_binary_format_v<T>,
            "Your value_type is not supported by bmp::binary_write");

        using pixel::channel_count_v;
        using value_type = pixel::channel_type_t<T>;

        static_assert(sizeof(value_type) <= 256);
        static_assert(sizeof(T) == sizeof(value_type) * channel_count_v<T>);
        if constexpr(!detail::endian_supported<value_type>) {
            if(endianness != std::endian::native) {
                throw std::runtime_error("endian conversion is not supported for requested type");
            }
        }

        // header informations
        std::uint8_t const version = 0x00;
        std::uint8_t const size_in_byte = sizeof(value_type);
        std::uint8_t const channel_count = channel_count_v<T>;
        auto const endian_flag = [endianness] {
            switch(endianness) {
            case std::endian::little:
                return detail::binary_endian_flags::is_little_endian;
            case std::endian::big:
                return detail::binary_endian_flags::is_big_endian;
            default:
                throw std::logic_error(
                    "unknown std::endian: " + std::to_string(std::uint32_t(endianness)));
            }
        }();
        std::uint8_t const flags = [endian_flag]() -> std::uint8_t {
            return (detail::binary_io_flags_v<value_type> & 0x0F) | std::uint8_t(endian_flag);
        }();

        std::uint64_t const w_bytes = detail::byteswap_on_little_endian(bitmap.w());
        std::uint64_t const h_bytes = detail::byteswap_on_little_endian(bitmap.h());

        // write the file header
        os.write(reinterpret_cast<char const*>(&detail::big_endian_io_magic), 4);
        os.write(reinterpret_cast<char const*>(&version), 1);
        os.write(reinterpret_cast<char const*>(&size_in_byte), 1);
        os.write(reinterpret_cast<char const*>(&channel_count), 1);
        os.write(reinterpret_cast<char const*>(&flags), 1);
        os.write(reinterpret_cast<char const*>(&w_bytes), 8);
        os.write(reinterpret_cast<char const*>(&h_bytes), 8);

        if(!os.good()) {
            throw binary_io_error("can't write binary bitmap format header");
        }

        if constexpr(std::is_same_v<T, bool>) {
            uint8_t data = 0;
            std::size_t i = 0;
            for(bool v: bitmap) {
                data <<= 1;
                data |= v ? 1 : 0;
                if(++i % 8 == 0) {
                    os.write(reinterpret_cast<char const*>(&data), 1);
                    data = 0;
                }
            }
            if(i % 8 != 0) {
                data <<= (8 - (i % 8));
                os.write(reinterpret_cast<char const*>(&data), 1);
            }
        } else if(endianness == std::endian::native) {
            os.write(reinterpret_cast<char const*>(bitmap.data()), bitmap.point_count() * sizeof(T));
        } else {
            for(auto const v: bitmap) {
                std::array<detail::integer_type<value_type>, channel_count_v<T>> buffer;
                for(std::size_t i = 0; i < buffer.size(); ++i) {
                    buffer[i] = detail::byteswap_to_integer(*(reinterpret_cast<value_type const*>(&v) + i));
                }
                os.write(reinterpret_cast<char const*>(buffer.data()), sizeof(T));
            }
        }

        if(!os.good()) {
            throw binary_io_error("can't write binary bitmap format data");
        }
    }


    /// \brief Write bitmap to disk by a given filename
    ///
    /// \throw binary_io_error
    template <typename T>
    void binary_write(
        bitmap<T> const& bitmap,
        std::string const& filename,
        std::endian endianness = std::endian::native) {
        std::ofstream os(filename.c_str(), std::ios_base::binary);

        if(!os.is_open()) {
            throw binary_io_error("can't open file: " + filename);
        }

        try {
            binary_write(bitmap, os, endianness);
        } catch(binary_io_error const& e) {
            throw binary_io_error(std::string(e.what()) + ": " + filename);
        }
    }


}
