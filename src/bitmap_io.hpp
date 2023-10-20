#pragma once

#include "bitmap.hpp"
#include "size_io.hpp"


namespace bmp {


    template <typename CharT, typename Traits, typename T>
    std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, bitmap<T> const& image) {
        os << '[' << image.size() << "]{";
        for(std::size_t y = 0; y < image.h(); ++y){
            if(y > 0){
                os << ", ";
            }

            os << "{";
            for(std::size_t x = 0; x < image.w(); ++x){
                if(x > 0){
                    os << ", ";
                }

                if constexpr(detail::is_char_v<T>) {
                    os << static_cast<int>(image(x, y));
                } else {
                    os << image(x, y);
                }
            }
            os << "}";
        }
        os << "}";

        return os;
    }


}
