#pragma once

#include <stdexcept>


namespace bmp {


    template <typename T>
    struct matrix3x3 {
        T d[3][3];
    };

    template <typename T, std::size_t Y, std::size_t X>
    void swap_lines(T (&m)[Y][X], std::size_t y1, std::size_t y2) {
        for(std::size_t i = 0; i < X; ++i) {
            using std::swap;
            swap(m[y1][i], m[y2][i]);
        }
    }

    template <typename T, std::size_t Y, std::size_t X>
    void gaussian_elimination(T (&m)[Y][X]) {
        static_assert(X > Y);

        for(std::size_t i = 0; i < Y; ++i) {
            if(m[i][i] == 0) {
                std::size_t y = i + 1;
                for(; y < Y; ++y) {
                    if(m[y][i] != 0) {
                        swap_lines(m, i, y);
                        break;
                    }
                }
                if(y == Y) {
                    throw std::logic_error("can't invert matrix");
                }
            }

            for(std::size_t x = i + 1; x < X; ++x) {
                m[i][x] /= m[i][i];
            }
            m[i][i] = 1;

            for(std::size_t y = i + 1; y < Y; ++y) {
                for(std::size_t x = i + 1; x < X; ++x) {
                    m[y][x] -= m[y][i] * m[i][x];
                }
                m[y][i] = 0;
            }
        }

        for(std::size_t i = 2; i > 0; --i) {
            for(std::size_t y = 0; y < i; ++y) {
                for(std::size_t x = Y; x < X; ++x) {
                    m[y][x] -= m[y][i] * m[i][x];
                }
            }
        }
    }

    template <typename T>
    constexpr matrix3x3<T> invert(matrix3x3<T> const& d) {
        T m[3][6]
            = {{d.d[0][0], d.d[0][1], d.d[0][2], 1, 0, 0},
               {d.d[1][0], d.d[1][1], d.d[1][2], 0, 1, 0},
               {d.d[2][0], d.d[2][1], d.d[2][2], 0, 0, 1}};

        gaussian_elimination(m);

        return matrix3x3<T>{
            {{m[0][3], m[0][4], m[0][5]},
             {m[1][3], m[1][4], m[1][5]},
             {m[2][3], m[2][4], m[2][5]}}};
    }


}
