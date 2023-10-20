#pragma once

#include "detail/io.hpp"

#include "size.hpp"


namespace bmp {


    template <typename CharT, typename Traits, typename WT, typename HT>
    std::basic_ostream<CharT, Traits>&
        operator<<(std::basic_ostream<CharT, Traits>& os, size<WT, HT> const& data) {
        if constexpr(detail::is_char_v<WT>) {
            os << static_cast<int>(data.w());
        } else {
            os << data.w();
        }

        os << "x";

        if constexpr(detail::is_char_v<HT>) {
            os << static_cast<int>(data.h());
        } else {
            os << data.h();
        }

        return os;
    }


    template <typename CharT, typename Traits, typename WT, typename HT>
    std::basic_istream<CharT, Traits>&
        operator>>(std::basic_istream<CharT, Traits>& is, size<WT, HT>& data) {
        size<WT, HT> tmp;
        is >> detail::input_wrapper(tmp.w());
        if(!detail::expect(is, 'x'))
            return is;
        is >> detail::input_wrapper(tmp.h());

        if(is) {
            data = std::move(tmp);
        }

        return is;
    }


}
