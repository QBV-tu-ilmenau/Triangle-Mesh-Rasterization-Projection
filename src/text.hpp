#pragma once

#include <algorithm>
#include <ranges>
#include <string_view>
#include <vector>


namespace ply::detail{


    using namespace std::literals;

    constexpr bool isspace(char const c){
        switch(c){
            case ' ' :
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                return true;
            default:
                return false;
        }
    }

    constexpr std::string_view trim_right(std::string_view const text){
        return {text.begin(), std::ranges::find_if_not(std::views::reverse(text), isspace).base()};
    }

    constexpr std::string_view trim_left(std::string_view const text){
        return {std::ranges::find_if_not(text, isspace), text.end()};
    }

    constexpr std::string_view trim(std::string_view const text){
        return trim_left(trim_right(text));
    }

    template <std::size_t N = 2>
    constexpr std::array<std::string_view, N> split_front(std::string_view line){
        static_assert(N > 0);

        std::array<std::string_view, N> result;

        for(std::size_t i = 0; i < N - 1; ++i){
            auto const iter = std::ranges::find_if(line, isspace);
            result[i] = {line.begin(), iter};
            line = trim_left({iter, line.end()});
        }
        result[N - 1] = line;

        return result;
    }

    template <std::size_t N = 2>
    constexpr std::array<std::string_view, N> split_back(std::string_view line){
        static_assert(N > 0);

        std::array<std::string_view, N> result;

        for(std::size_t i = 0; i < N - 1; ++i){
            auto const iter = std::ranges::find_if(std::views::reverse(line), isspace).base();
            result[N - i - 1] = {iter, line.end()};
            line = trim_right({line.begin(), iter});
        }
        result[0] = line;

        return result;
    }


    static_assert(trim_right("abc"sv) == "abc"sv);
    static_assert(trim_right(" abc "sv) == " abc"sv);
    static_assert(trim_right("  abc  "sv) == "  abc"sv);

    static_assert(trim_left("abc"sv) == "abc"sv);
    static_assert(trim_left(" abc "sv) == "abc "sv);
    static_assert(trim_left("  abc  "sv) == "abc  "sv);

    static_assert(trim("abc"sv) == "abc"sv);
    static_assert(trim(" abc "sv) == "abc"sv);
    static_assert(trim("  abc  "sv) == "abc"sv);


    static_assert(split_front<1>(" abc "sv) == std::array{" abc "sv});
    static_assert(split_front<1>("  abc  "sv) == std::array{"  abc  "sv});
    static_assert(split_front<1>("abc"sv) == std::array{"abc"sv});
    static_assert(split_front<1>("abc def"sv) == std::array{"abc def"sv});
    static_assert(split_front<1>("a b c"sv) == std::array{"a b c"sv});
    static_assert(split_front<1>(" a b c "sv) == std::array{" a b c "sv});

    static_assert(split_front(" abc "sv) == std::array{""sv, "abc "sv});
    static_assert(split_front("  abc  "sv) == std::array{""sv, "abc  "sv});
    static_assert(split_front("abc"sv) == std::array{"abc"sv, ""sv});
    static_assert(split_front("abc def"sv) == std::array{"abc"sv, "def"sv});
    static_assert(split_front("a b c"sv) == std::array{"a"sv, "b c"sv});
    static_assert(split_front(" a b c "sv) == std::array{""sv, "a b c "sv});

    static_assert(split_front<3>(" abc "sv) == std::array{""sv, "abc"sv, ""sv});
    static_assert(split_front<3>("  abc  "sv) == std::array{""sv, "abc"sv, ""sv});
    static_assert(split_front<3>("abc"sv) == std::array{"abc"sv, ""sv, ""sv});
    static_assert(split_front<3>("abc def"sv) == std::array{"abc"sv, "def"sv, ""sv});
    static_assert(split_front<3>("a b c"sv) == std::array{"a"sv, "b"sv, "c"sv});
    static_assert(split_front<3>(" a b c "sv) == std::array{""sv, "a"sv, "b c "sv});


    static_assert(split_back<1>(" abc "sv) == std::array{" abc "sv});
    static_assert(split_back<1>("  abc  "sv) == std::array{"  abc  "sv});
    static_assert(split_back<1>("abc"sv) == std::array{"abc"sv});
    static_assert(split_back<1>("abc def"sv) == std::array{"abc def"sv});
    static_assert(split_back<1>("a b c"sv) == std::array{"a b c"sv});
    static_assert(split_back<1>(" a b c "sv) == std::array{" a b c "sv});

    static_assert(split_back(" abc "sv) == std::array{" abc"sv, ""sv});
    static_assert(split_back("  abc  "sv) == std::array{"  abc"sv, ""sv});
    static_assert(split_back("abc"sv) == std::array{""sv, "abc"sv});
    static_assert(split_back("abc def"sv) == std::array{"abc"sv, "def"sv});
    static_assert(split_back("a b c"sv) == std::array{"a b"sv, "c"sv});
    static_assert(split_back(" a b c "sv) == std::array{" a b c"sv, ""sv});

    static_assert(split_back<3>(" abc "sv) == std::array{""sv, "abc"sv, ""sv});
    static_assert(split_back<3>("  abc  "sv) == std::array{""sv, "abc"sv, ""sv});
    static_assert(split_back<3>("abc"sv) == std::array{""sv, ""sv, "abc"sv});
    static_assert(split_back<3>("abc def"sv) == std::array{""sv, "abc"sv, "def"sv});
    static_assert(split_back<3>("a b c"sv) == std::array{"a"sv, "b"sv, "c"sv});
    static_assert(split_back<3>(" a b c "sv) == std::array{" a b"sv, "c"sv, ""sv});


}
