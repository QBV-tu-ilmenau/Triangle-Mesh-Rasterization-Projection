#pragma once

#include <sstream>


namespace bmp::detail {


    template <typename T>
    constexpr bool is_char_v
        = std::is_same_v<
              T,
              char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>;


    inline bool extract_if_is(std::istream& is, char const should_be) {
        if(is) {
            if(char in; is >> in) {
                if(in == should_be) {
                    return true;
                }
                is.putback(in);
            } else if(is.eof()) {
                is.clear();
            }
        }
        return false;
    }

    template <typename ShouldBe>
    bool expect(std::istream& is, ShouldBe const& should_be) {
        if(is) {
            auto const pos = is.tellg();
            ShouldBe in;
            if(is >> in) {
                if(in == should_be) {
                    return true;
                }
                is.seekg(pos);
            } else if(is.eof()) {
                is.clear();
                is.seekg(pos);
            }

            is.setstate(std::ios_base::failbit);
        }
        return false;
    }

    inline bool expect(std::istream& is, char const should_be) {
        if(auto const result = extract_if_is(is, should_be); !result) {
            is.setstate(std::ios_base::failbit);
        } else {
            return result;
        }
    }


    template <typename T>
    struct input_wrapper_t {
        input_wrapper_t(T& value)
            : value(value) {}
        input_wrapper_t(input_wrapper_t<T> const& c)
            : value(c.value) {}

        T& value;
    };

    template <typename T>
    decltype(auto) input_wrapper(T& value) {
        if constexpr(std::is_floating_point_v<T>) {
            return input_wrapper_t<T>{value};
        } else {
            return (value);
        }
    }


    template <typename CharT, typename Traits, typename T>
    std::basic_istream<CharT, Traits>&
        operator>>(std::basic_istream<CharT, Traits>& is, input_wrapper_t<T> data) {
        // float followed by characters (like in 1024x768) don't work,
        // therefor this reads all allowed character before parsing.
        // Attention: Only floats in fixed formatting are implemented

        std::stringstream buffer;
        CharT in;
        if(is) {
            bool point_found = false;
            while(is >> in) {
                if((in >= '0' && in <= '9') || (!point_found && in == '.')) {
                    buffer << in;
                    if(!point_found && in == '.') {
                        point_found = true;
                    }
                } else {
                    is.putback(in);
                    break;
                }
            }

            if(is) {
                buffer >> data.value;
            }
        }

        return is;
    }


}
