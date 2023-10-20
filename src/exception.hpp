#pragma once

#include <stdexcept>


namespace bmp {


    /// \brief Exception class for binary_write and binary_read bitmaps
    struct binary_io_error: std::runtime_error {
        using std::runtime_error::runtime_error;
    };


}
