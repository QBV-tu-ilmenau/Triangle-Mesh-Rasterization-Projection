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


    struct binary_header {
        std::uint8_t version;
        std::uint8_t channel_size;
        std::uint8_t channel_count;
        std::uint8_t flags;
        std::uint64_t w;
        std::uint64_t h;
    };

    /// \brief Read binary bitmap format header from std::istream
    ///
    /// \throw binary_io_error
    inline binary_header binary_read_header(std::istream& is) {
        // read the file header
        uint32_t magic;
        is.read(reinterpret_cast<char*>(&magic), 4);
        if(magic != detail::big_endian_io_magic) {
            throw binary_io_error("wrong magic number");
        }

        uint8_t version;
        is.read(reinterpret_cast<char*>(&version), 1);
        if(version != 0x00) {
            throw binary_io_error(
                "file format version is " + std::to_string(version)
                + ", but only version 0 is supported");
        }

        std::uint8_t channel_size;
        std::uint8_t channel_count;
        std::uint8_t flags;
        std::uint64_t w_bytes;
        std::uint64_t h_bytes;

        is.read(reinterpret_cast<char*>(&channel_size), 1);
        is.read(reinterpret_cast<char*>(&channel_count), 1);
        is.read(reinterpret_cast<char*>(&flags), 1);
        is.read(reinterpret_cast<char*>(&w_bytes), 8);
        is.read(reinterpret_cast<char*>(&h_bytes), 8);

        w_bytes = detail::byteswap_on_little_endian(w_bytes);
        h_bytes = detail::byteswap_on_little_endian(h_bytes);

        if(!is.good()) {
            throw binary_io_error("can't read binary bitmap format header");
        }

        return {version, channel_size, channel_count, flags, w_bytes, h_bytes};
    }

    /// \brief Read binary bitmap format data from std::istream
    ///
    /// \throw binary_io_error
    template <typename T>
    void binary_read_data(
        bitmap<T>& bitmap,
        std::istream& is,
        binary_header const& header,
        bool ignore_signed = true) {
        static_assert(
            detail::is_valid_binary_format_v<T>,
            "Your value_type is not supported by bmp::binary_read");

        using pixel::channel_count_v;
        using value_type = pixel::channel_type_t<T>;
        using detail::binary_endian_flags;
        using detail::binary_type_flags;

        static_assert(sizeof(value_type) <= 256);
        static_assert(sizeof(T) == sizeof(value_type) * channel_count_v<T>);

        auto const fix_flag = [ignore_signed](binary_type_flags flag) {
            if(!ignore_signed)
                return flag;
            if(flag == binary_type_flags::is_signed) {
                return binary_type_flags::is_unsigned;
            }
            return flag;
        };


        std::uint8_t const ref_flags = detail::binary_io_flags_v<value_type>;

        auto const ref_type_flag = fix_flag(binary_type_flags(ref_flags & 0x0F));
        auto const ref_endian_flag = binary_endian_flags(ref_flags & 0xF0);

        auto const test_type_flag = fix_flag(binary_type_flags(header.flags & 0x0F));
        auto const test_endian_flag = binary_endian_flags(header.flags & 0xF0);


        if(test_type_flag != ref_type_flag) {
            auto const print_type_flag = [ignore_signed](binary_type_flags flag) {
                switch(flag) {
                case binary_type_flags::is_unsigned:
                    if(ignore_signed)
                        return "integer";
                    return "unsigned integer";
                case binary_type_flags::is_signed:
                    if(ignore_signed)
                        return "integer";
                    return "signed integer";
                case binary_type_flags::is_floating_point:
                    return "floating point";
                case binary_type_flags::is_bool:
                    return "bool";
                default:
                    throw std::logic_error(
                        "unknown binary_type_flags flag: " + std::to_string(std::uint32_t(flag)));
                }
            };

            std::ostringstream is;
            is << "wrong type " << print_type_flag(test_type_flag) << ", expected "
               << print_type_flag(ref_type_flag);
            throw binary_io_error(is.str());
        }


        if(header.channel_size != sizeof(value_type)) {
            throw binary_io_error(
                "wrong value_type size " + std::to_string(header.channel_size) + ", expected "
                + std::to_string(sizeof(value_type)));
        }

        if(header.channel_count != channel_count_v<T>) {
            throw binary_io_error(
                "wrong channel count " + std::to_string(header.channel_count) + ", expected "
                + std::to_string(channel_count_v<T>));
        }

        auto const print_endian = [](binary_endian_flags flag) {
            using namespace std::literals::string_literals;
            switch(flag) {
            case binary_endian_flags::is_big_endian:
                return "big"s;
            case binary_endian_flags::is_little_endian:
                return "little"s;
            default:
                throw std::logic_error(
                    "unknown binary_endian_flags flag: " + std::to_string(std::uint32_t(flag)));
            }
        };

        if constexpr(!detail::endian_supported<value_type>) {
            if(test_endian_flag != ref_endian_flag) {
                throw std::runtime_error("data in "
                    + print_endian(test_endian_flag) + " endian, expected "
                    + print_endian(ref_endian_flag) + " endian, conversion is "
                    "not supported for requested type");
            }
        } else {
            print_endian(test_endian_flag); // throws if not valid
        }


        bitmap.resize(header.w, header.h);
        auto pixel_count = bitmap.point_count();
        if constexpr(std::is_same_v<T, bool>) {
            std::vector<char> buffer((pixel_count + 7) / 8);
            is.read(buffer.data(), buffer.size());
            for(std::size_t i = 0; i < pixel_count; ++i) {
                *(bitmap.begin() + i) = (buffer[i / 8] & (1 << (7 - (i % 8)))) != 0;
            }
        } else if(test_endian_flag == ref_endian_flag) {
            is.read(reinterpret_cast<char*>(bitmap.data()), pixel_count * sizeof(T));
        } else if constexpr(std::integral<value_type>) {
            is.read(reinterpret_cast<char*>(bitmap.data()), pixel_count * sizeof(T));

            // fix endianness
            for(auto& v: bitmap) {
                for(std::size_t i = 0; i < channel_count_v<T>; ++i) {
                    auto& c = *(reinterpret_cast<value_type*>(&v) + i);
                    c = std::byteswap(c);
                }
            }
        } else {
            using buffer_value_type = detail::integer_type<value_type>;
            using buffer_type = std::array<buffer_value_type, channel_count_v<T>>;
            bmp::bitmap<buffer_type> buffer(header.w, header.h);

            is.read(reinterpret_cast<char*>(buffer.data()), pixel_count * sizeof(T));

            // fix endianness
            std::ranges::transform(buffer, bitmap.begin(),
                [](buffer_type const& channels){
                    T pixel;
                    for(std::size_t i = 0; i < channel_count_v<T>; ++i) {
                        *(reinterpret_cast<value_type*>(&pixel) + i) = detail::byteswap_to<value_type>(channels[i]);
                    }
                    return pixel;
                });
        }

        if(!is.good()) {
            throw binary_io_error("can't read binary bitmap format data");
        }
    }

    /// \brief Read binary bitmap format data from std::istream
    ///
    /// \throw binary_io_error
    template <typename T>
    bitmap<T>
        binary_read_data(std::istream& is, binary_header const& header, bool ignore_signed = true) {
        bitmap<T> bitmap;
        binary_read_data(bitmap, is, header, ignore_signed);
        return bitmap;
    }

    /// \brief Read bitmap from std::istream
    ///
    /// \throw binary_io_error
    template <typename T>
    void binary_read(bitmap<T>& bitmap, std::istream& is, bool ignore_signed = true) {
        auto const header = binary_read_header(is);
        binary_read_data(bitmap, is, header, ignore_signed);
    }

    /// \brief Read bitmap from std::istream
    ///
    /// \throw binary_io_error
    template <typename T>
    bitmap<T> binary_read(std::istream& is, bool ignore_signed = true) {
        bitmap<T> bitmap;
        binary_read(bitmap, is, ignore_signed);
        return bitmap;
    }

    /// \brief Read bitmap from disk by a given filename
    ///
    /// \throw binary_io_error
    template <typename T>
    void binary_read(bitmap<T>& bitmap, std::string const& filename, bool ignore_signed = true) {
        std::ifstream is(filename.c_str(), std::ios_base::binary);

        if(!is.is_open()) {
            throw binary_io_error("can't open file: " + filename);
        }

        try {
            binary_read(bitmap, is, ignore_signed);
        } catch(binary_io_error const& error) {
            throw binary_io_error(std::string(error.what()) + ": " + filename);
        }
    }

    /// \brief Read bitmap from disk by a given filename
    ///
    /// \throw binary_io_error
    template <typename T>
    bitmap<T> binary_read(std::string const& filename, bool ignore_signed = true) {
        bitmap<T> bitmap;
        binary_read(bitmap, filename, ignore_signed);
        return bitmap;
    }


}
