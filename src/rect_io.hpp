#pragma once

#include "point_io.hpp"
#include "rect.hpp"
#include "size_io.hpp"


namespace bmp {


    template <typename CharT, typename Traits, typename XT, typename YT, typename WT, typename HT>
    std::basic_ostream<CharT, Traits>&
        operator<<(std::basic_ostream<CharT, Traits>& os, rect<XT, YT, WT, HT> const& data) {
        return os << data.pos() << ":" << data.size();
    }


}
