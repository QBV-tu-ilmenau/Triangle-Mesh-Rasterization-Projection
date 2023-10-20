#pragma once

#include "detail/io.hpp"

#include "point.hpp"


namespace bmp {


    template <typename CharT, typename Traits, typename XT, typename YT>
    std::basic_ostream<CharT, Traits>&
        operator<<(std::basic_ostream<CharT, Traits>& os, point<XT, YT> const& data) {
        if constexpr(detail::is_char_v<XT>) {
            os << static_cast<int>(data.x());
        } else {
            os << data.x();
        }

        os << "x";

        if constexpr(detail::is_char_v<YT>) {
            os << static_cast<int>(data.y());
        } else {
            os << data.y();
        }

        return os;
    }


    template <typename CharT, typename Traits, typename XT, typename YT>
    std::basic_istream<CharT, Traits>&
        operator>>(std::basic_istream<CharT, Traits>& is, point<XT, YT>& data) {
        point<XT, YT> tmp;
        is >> detail::input_wrapper(tmp.x());
        if(!detail::expect(is, 'x'))
            return is;
        is >> detail::input_wrapper(tmp.y());

        if(is) {
            data = std::move(tmp);
        }

        return is;
    }


}
