#pragma once

#include <bitmap/bitmap.hpp>
#include <bitmap/pixel.hpp>
#include <bitmap/masked_pixel.hpp>

#include <boost/endian.hpp>

#include <png.h>

#include <concepts>
#include <csetjmp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>


namespace bmp::png{


    using namespace std::literals;

    struct g1{
        using value_type = bool;
        static constexpr int channels = PNG_COLOR_TYPE_GRAY;
        static constexpr int bit_depth = 1;

        value_type g;
    };

    struct g8{
        using value_type = boost::endian::big_uint8_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_GRAY;
        static constexpr int bit_depth = 8;

        boost::endian::big_uint8_buf_t g;
    };

    struct g16{
        using value_type = boost::endian::big_uint16_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_GRAY;
        static constexpr int bit_depth = 16;

        boost::endian::big_uint16_buf_t g;
    };

    struct ga8{
        using value_type = boost::endian::big_uint8_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_GRAY_ALPHA;
        static constexpr int bit_depth = 8;

        boost::endian::big_uint8_buf_t g;
        boost::endian::big_uint8_buf_t a;
    };

    struct ga16{
        using value_type = boost::endian::big_uint16_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_GRAY_ALPHA;
        static constexpr int bit_depth = 16;

        boost::endian::big_uint16_buf_t g;
        boost::endian::big_uint16_buf_t a;
    };

    struct rgb8{
        using value_type = boost::endian::big_uint8_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_RGB;
        static constexpr int bit_depth = 8;

        boost::endian::big_uint8_buf_t r;
        boost::endian::big_uint8_buf_t g;
        boost::endian::big_uint8_buf_t b;
    };

    struct rgb16{
        using value_type = boost::endian::big_uint16_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_RGB;
        static constexpr int bit_depth = 16;

        boost::endian::big_uint16_buf_t r;
        boost::endian::big_uint16_buf_t g;
        boost::endian::big_uint16_buf_t b;
    };

    struct rgba8{
        using value_type = boost::endian::big_uint8_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_RGB_ALPHA;
        static constexpr int bit_depth = 8;

        boost::endian::big_uint8_buf_t r;
        boost::endian::big_uint8_buf_t g;
        boost::endian::big_uint8_buf_t b;
        boost::endian::big_uint8_buf_t a;
    };

    struct rgba16{
        using value_type = boost::endian::big_uint16_buf_t;
        static constexpr int channels = PNG_COLOR_TYPE_RGB_ALPHA;
        static constexpr int bit_depth = 16;

        boost::endian::big_uint16_buf_t r;
        boost::endian::big_uint16_buf_t g;
        boost::endian::big_uint16_buf_t b;
        boost::endian::big_uint16_buf_t a;
    };


    template <typename T, typename ... U>
    concept is_same_as_any_of = (std::same_as<T, U> || ...);

    template <typename T>
    concept write_pixel = is_same_as_any_of<T, g1, g8, g16, ga8, ga16, rgb8, rgb16, rgba8, rgba16>;

    template <typename T>
    concept g_pixel = is_same_as_any_of<T, g1, g8, g16>;

    template <typename T>
    concept ga_pixel = is_same_as_any_of<T, ga8, ga16>;

    template <typename T>
    concept rgb_pixel = is_same_as_any_of<T, rgb8, rgb16>;

    template <typename T>
    concept rgba_pixel = is_same_as_any_of<T, rgba8, rgba16>;


    template <g_pixel To>
    struct access_g{
        using target_type = To;

        template <typename From>
        constexpr To operator()(From const& pixel)const{
            return To(static_cast<typename To::value_type>(pixel));
        }
    };

    template <ga_pixel To>
    struct access_ga{
        using target_type = To;

        template <typename From>
        constexpr To operator()(From const& pixel)const{
            return To(
                static_cast<typename To::value_type>(pixel.g),
                static_cast<typename To::value_type>(pixel.a));
        }

        template <typename From>
        constexpr To operator()(pixel::basic_masked_pixel<From> const& pixel)const{
            return To(
                static_cast<typename To::value_type>(pixel.v),
                static_cast<typename To::value_type>(
                    pixel.m ? 0 : std::numeric_limits<typename To::value_type::value_type>::max()));
        }
    };

    template <rgb_pixel To>
    struct access_rgb{
        using target_type = To;

        template <typename From>
        constexpr To operator()(From const& pixel)const{
            return To(
                static_cast<typename To::value_type>(pixel.r),
                static_cast<typename To::value_type>(pixel.g),
                static_cast<typename To::value_type>(pixel.b));
        }
    };

    template <rgba_pixel To>
    struct access_rgba{
        using target_type = To;

        template <typename From>
        constexpr To operator()(From const& pixel)const{
            return To(
                static_cast<typename To::value_type>(pixel.r),
                static_cast<typename To::value_type>(pixel.g),
                static_cast<typename To::value_type>(pixel.b),
                static_cast<typename To::value_type>(pixel.a));
        }

        template <typename From>
        constexpr To operator()(pixel::basic_masked_pixel<From> const& pixel)const{
            return To(
                static_cast<typename To::value_type>(pixel.v.r),
                static_cast<typename To::value_type>(pixel.v.g),
                static_cast<typename To::value_type>(pixel.v.b),
                static_cast<typename To::value_type>(
                    pixel.m ? 0 : std::numeric_limits<typename To::value_type::value_type>::max()));
        }
    };


    struct pixel_type_not_supported_by_png{};


    template <typename Pixel> inline constexpr auto pixel_access = pixel_type_not_supported_by_png{};

    template <> inline constexpr auto pixel_access<bool> = access_g<g1>{};

    template <> inline constexpr auto pixel_access<std::int8_t> = access_g<g8>{};
    template <> inline constexpr auto pixel_access<std::uint8_t> = access_g<g8>{};
    template <> inline constexpr auto pixel_access<std::int16_t> = access_g<g16>{};
    template <> inline constexpr auto pixel_access<std::uint16_t> = access_g<g16>{};

    template <> inline constexpr auto pixel_access<pixel::ga8> = access_ga<ga8>{};
    template <> inline constexpr auto pixel_access<pixel::ga8u> = access_ga<ga8>{};
    template <> inline constexpr auto pixel_access<pixel::ga16> = access_ga<ga16>{};
    template <> inline constexpr auto pixel_access<pixel::ga16u> = access_ga<ga16>{};

    template <> inline constexpr auto pixel_access<pixel::rgb8> = access_rgb<rgb8>{};
    template <> inline constexpr auto pixel_access<pixel::rgb8u> = access_rgb<rgb8>{};
    template <> inline constexpr auto pixel_access<pixel::rgb16> = access_rgb<rgb16>{};
    template <> inline constexpr auto pixel_access<pixel::rgb16u> = access_rgb<rgb16>{};

    template <> inline constexpr auto pixel_access<pixel::rgba8> = access_rgba<rgba8>{};
    template <> inline constexpr auto pixel_access<pixel::rgba8u> = access_rgba<rgba8>{};
    template <> inline constexpr auto pixel_access<pixel::rgba16> = access_rgba<rgba16>{};
    template <> inline constexpr auto pixel_access<pixel::rgba16u> = access_rgba<rgba16>{};

    template <> inline constexpr auto pixel_access<pixel::masked_g8> = access_ga<ga8>{};
    template <> inline constexpr auto pixel_access<pixel::masked_g8u> = access_ga<ga8>{};
    template <> inline constexpr auto pixel_access<pixel::masked_g16> = access_ga<ga16>{};
    template <> inline constexpr auto pixel_access<pixel::masked_g16u> = access_ga<ga16>{};

    template <> inline constexpr auto pixel_access<pixel::masked_rgb8> = access_rgba<rgba8>{};
    template <> inline constexpr auto pixel_access<pixel::masked_rgb8u> = access_rgba<rgba8>{};
    template <> inline constexpr auto pixel_access<pixel::masked_rgb16> = access_rgba<rgba16>{};
    template <> inline constexpr auto pixel_access<pixel::masked_rgb16u> = access_rgba<rgba16>{};


    inline std::string quoted(std::string const& text){
        std::ostringstream os;
        os << std::quoted(text);
        return os.str();
    }


    class writer{
    public:
        writer() = default;

        writer(writer const&) = delete;
        writer& operator=(writer const&) = delete;

        ~writer()noexcept{
            reset();
        }

        template <typename T>
        bool write(bitmap<T> const& image, std::filesystem::path const& filepath){
            std::ofstream os(filepath, std::ios::binary);
            if(!os.is_open()){
                throw std::runtime_error("can not open file " + quoted(filepath.string()));
            }

            return write(image, os);
        }

        template <typename T>
        bool write(bitmap<T> const& image, std::ostream& os)noexcept{
            static constexpr auto max_size = static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max());
            if(image.h() > max_size || image.h() > max_size){
                if(error_callable_){
                    error_callable_("dimensions are to large for PNG file format"sv);
                }
                return false;
            }

            reset();

            main_ = ::png_create_write_struct(PNG_LIBPNG_VER_STRING, this, &error, &warn);
            if(!main_){
                if(error_callable_){
                    error_callable_("failed to allocate png struct"sv);
                }
                return false;
            }

            info_ = ::png_create_info_struct(main_);
            if(!info_){
                if(error_callable_){
                    error_callable_("failed to allocate png info struct"sv);
                }
                return false;
            }

            // libpng will jump to this condition if an error occurs
            if(setjmp(png_jmpbuf(main_))){
                return false;
            }

            ::png_set_write_fn(main_, &os, &write_data, &flush_data);

            auto const w = image.template w_as<std::int32_t>();
            auto const h = image.template h_as<std::int32_t>();

            using pixel_type = decltype(pixel_access<T>)::target_type;
            ::png_set_IHDR(main_, info_, w, h, pixel_type::bit_depth, pixel_type::channels,
                PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

            // TODO: Implement some direct access instead of copy for types which support this
            std::vector<pixel_type> data;
            data.reserve(image.point_count());
            std::ranges::transform(image, std::back_inserter(data),
                [](T const& v){
                    return pixel_access<T>(v);
                });

            std::vector<::png_byte*> rows;
            rows.reserve(h);
            for(std::size_t y = 0; y < static_cast<std::size_t>(h); ++y){
                rows.push_back(reinterpret_cast<::png_byte*>(&data[y * w]));
            }

            ::png_set_rows(main_, info_, rows.data());

            ::png_write_png(main_, info_, PNG_TRANSFORM_IDENTITY, nullptr);

            return true;
        }

        void on_error(std::function<void(std::string_view)> callable)noexcept{
            error_callable_ = std::move(callable);
        }

        void on_warning(std::function<void(std::string_view)> callable)noexcept{
            warning_callable_ = std::move(callable);
        }

    private:
        void reset(){
            png_destroy_write_struct(&main_, &info_);

            main_ = nullptr;
            info_ = nullptr;
        }

        [[noreturn]] static void error(::png_struct* main, char const* message)noexcept{
            auto const& callable = static_cast<writer const*>(::png_get_error_ptr(main))->error_callable_;
            if(callable){
                callable(std::string_view(message));
            }

            // libpng requires the error handler to jump back to our write routine
            std::longjmp(png_jmpbuf(main), -1);
        }

        static void warn(::png_struct* main, char const* message)noexcept{
            auto const& callable = static_cast<writer const*>(::png_get_error_ptr(main))->warning_callable_;
            if(callable){
                callable(std::string_view(message));
            }
        }

        static void write_data(::png_struct* png, ::png_byte* data, ::png_size_t length)noexcept try{
            auto& os = *static_cast<std::ostream*>(::png_get_io_ptr(png));
            os.write(reinterpret_cast<char*>(data), length);
            if(!os.good()){
                error(png, "std::ostream::write() failed");
            }
        }catch(std::exception const& e){
            error(png, e.what());
        }catch(...){
            error(png, "write_data: unknown error");
        }

        static void flush_data(png_struct* png)noexcept try{
            auto& os = *static_cast<std::ostream*>(::png_get_io_ptr(png));
            os.flush();
            if(!os.good()){
                error(png, "std::ostream::flush() failed");
            }
        }catch(std::exception const& e){
            error(png, e.what());
        }catch(...){
            error(png, "flush_data: unknown error");
        }

        ::png_struct* main_ = nullptr;
        ::png_info* info_ = nullptr;

        std::function<void(std::string_view)> error_callable_;
        std::function<void(std::string_view)> warning_callable_;
    };

    template <typename T>
    bool write(bitmap<T> const& image, std::ostream& os)noexcept{
        return writer{}.write(image, os);
    }

    template <typename T>
    bool write(bitmap<T> const& image, std::filesystem::path const& filepath){
        return writer{}.write(image, filepath);
    }


}
