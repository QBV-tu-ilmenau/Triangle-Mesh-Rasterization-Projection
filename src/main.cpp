#include "ply.hpp"
#include "image_format_png.hpp"

#include <bitmap/bitmap.hpp>
#include <bitmap/binary_write.hpp>

#include <fmt/color.h>

#include <argparse/argparse.hpp>

#include <algorithm>
#include <numeric>
#include <span>
#include <tuple>


namespace ply2image{


    using namespace std::literals;

    inline constexpr auto NaN = std::numeric_limits<double>::quiet_NaN();


    enum class file_format{
        bbf = 0,
        png = 1
    };

    constexpr std::string_view file_format_strings[] = {"bbf"sv, "png"sv};


    enum class raster_filter{
        min = 0,
        max = 1,
        none = 2,
    };

    constexpr std::string_view raster_filter_strings[] = {"min"sv, "max"sv, "none"sv};


    std::string valid_values_string(std::ranges::output_range<std::string_view> auto const& list){
        auto const begin = std::ranges::begin(list);
        auto const end = std::ranges::end(list);
        if(begin == end){
            throw std::logic_error("no valid values");
        }

        auto result = "(valid values: "s;
        result += fmt::format("{:s}", std::quoted(*begin));
        for(auto const& entry: std::span(std::next(begin), end)){
            result += fmt::format(", {:s}", std::quoted(entry));
        }
        result += ")"s;
        return result;
    }

    template <typename T>
    requires std::is_enum_v<T>
    T parse_enum_string(std::ranges::output_range<std::string_view> auto const& list, std::string_view const value){
        auto const iter = std::ranges::find(list, value);
        if(iter == std::ranges::end(list)){
            throw std::runtime_error(
                fmt::format("invalid file format {:s} {:s}", value, valid_values_string(list)));
        }
        return static_cast<T>(iter - std::ranges::begin(list));
    }


    std::string trim_left(std::string_view const text, char const character){
        return {std::ranges::find_if(text, [=](char const c){ return c != character; }), text.end()};
    }


    constexpr bool contains(auto list, auto value){
        return std::ranges::find(list, value) != list.end();
    }


    template <typename T>
    std::optional<T> get_raster(
        argparse::ArgumentParser const& program,
        std::string_view const arg_name,
        std::string_view const disabled_name
    ){
        if(program.get<bool>(disabled_name)){
            if(program.is_used(arg_name)){
                throw std::runtime_error(
                    "You cannot use " + std::string(arg_name) + " together with " + std::string(disabled_name));
            }else{
                return std::nullopt;
            }
        }else{
            return program.get<T>(arg_name);
        }
    }

    template <ply::valid_value T>
    std::int64_t raster_convert(T const v){
        using limits = std::numeric_limits<std::int64_t>;
        if constexpr(ply::scalar_value<T>)[[likely]]{
            if constexpr(std::is_floating_point_v<T>){
                if(v != std::floor(v)){
                    throw std::runtime_error("raster property contains at least one non-integer value");
                }else if(v < static_cast<T>(limits::min()) || v > static_cast<T>(limits::max())){
                    throw std::runtime_error("raster property value is out of range");
                }
            }else if constexpr(std::is_same_v<T, std::uint64_t>){
                if(v > std::uint64_t(limits::max())){
                    throw std::runtime_error("raster property value is out of range");
                }
            }

            return static_cast<std::int64_t>(v);
        }else{
            throw std::runtime_error("list type properties are not supported");
        }
    }

    struct point{
        double x;
        double y;
        double v;
    };

    struct raster_point{
        double x;
        double y;
        double v;
        std::int64_t rx;
        std::int64_t ry;

        operator bmp::point<double>()const{
            return {x, y};
        };
    };

    template <typename Point>
    struct raw_pixel;

    template <>
    struct raw_pixel<point>{
        double weight;
        double value;
    };

    template <>
    struct raw_pixel<raster_point>{
        double weight;
        double value;
        std::int64_t rx;
        std::int64_t ry;
    };


    struct max_value_filter{
        constexpr auto operator()(std::vector<raw_pixel<raster_point>> const& p)const{
            return std::ranges::max_element(p, [](raw_pixel<raster_point> const& a, raw_pixel<raster_point> const& b){
                return a.value < b.value;
            });
        }
    };

    struct min_value_filter{
        constexpr auto operator()(std::vector<raw_pixel<raster_point>> const& p)const{
            return std::ranges::min_element(p, [](raw_pixel<raster_point> const& a, raw_pixel<raster_point> const& b){
                return a.value < b.value;
            });
        }
    };

    struct none_filter{};


    bmp::bitmap<std::vector<raw_pixel<point>>> to_vector_image(
        std::size_t const width,
        std::size_t const height,
        std::vector<point> const& points
    ){
        bmp::bitmap<std::vector<raw_pixel<point>>> vector_image(width, height);
        for(auto const& p: points){
            auto const x = p.x;
            auto const y = p.y;
            auto const ix = static_cast<std::size_t>(std::floor(x));
            auto const iy = static_cast<std::size_t>(std::floor(y));
            auto const xr = x - std::floor(x);
            auto const yr = y - std::floor(y);
            if(ix < vector_image.w() && iy < vector_image.h()){
                auto const weight = (1.f - xr) * (1.f - yr);
                vector_image(ix    , iy    ).push_back({weight, p.v});
            }
            if(ix + 1 < vector_image.w() && iy < vector_image.h()){
                auto const weight = (      xr) * (1.f - yr);
                vector_image(ix + 1, iy    ).push_back({weight, p.v});
            }
            if(ix < vector_image.w() && iy + 1 < vector_image.h()){
                auto const weight = (1.f - xr) * (      yr);
                vector_image(ix    , iy + 1).push_back({weight, p.v});
            }
            if(ix + 1 < vector_image.w() && iy + 1 < vector_image.h()){
                auto const weight = (      xr) * (      yr);
                vector_image(ix + 1, iy + 1).push_back({weight, p.v});
            }
        }
        return vector_image;
    }

    struct raster_range{
        std::int64_t min_x;
        std::int64_t max_x;
        std::int64_t min_y;
        std::int64_t max_y;

        std::size_t w()const{
            return static_cast<std::size_t>(max_x + 1 - min_x);
        }

        std::size_t h()const{
            return static_cast<std::size_t>(max_y + 1 - min_y);
        }

        std::size_t x(std::int64_t const x)const{
            return static_cast<std::size_t>(x - min_x);
        }

        std::size_t y(std::int64_t const y)const{
            return static_cast<std::size_t>(y - min_y);
        }
    };

    raster_range find_raster_range(std::vector<raster_point> const& points){
        using limits = std::numeric_limits<std::int64_t>;
        raster_range range{limits::max(), limits::min(), limits::max(), limits::min()};
        for(auto const& p: points){
            range.min_x = std::min(range.min_x, p.rx);
            range.max_x = std::max(range.max_x, p.rx);
            range.min_y = std::min(range.min_y, p.ry);
            range.max_y = std::max(range.max_y, p.ry);
        }
        return range;
    }


    constexpr double sqr(double const v)noexcept{
        return v * v;
    }

    constexpr double distance(bmp::point<double> const& a, bmp::point<double> const& b){
        return std::sqrt(sqr(a.x() - b.x()) + sqr(a.y() - b.y()));
    }

    constexpr double area(std::array<bmp::point<double>, 3> const& t){
        auto const a = distance(t[0], t[1]);
        auto const b = distance(t[1], t[2]);
        auto const c = distance(t[2], t[0]);
        auto const s = (a + b + c) / 2.;
        return std::sqrt(s * (s - a) * (s - b) * (s - a));
    }

    constexpr bool is_inside(std::array<raster_point, 3> const& t, bmp::point<double> const& p){
        constexpr auto sign =
            [](std::array<bmp::point<double>, 3> const& t){
                return (t[0].x() - t[2].x()) * (t[1].y() - t[2].y()) - (t[1].x() - t[2].x()) * (t[0].y() - t[2].y());
            };

        auto const d1 = sign({p, {t[0].x, t[0].y}, {t[1].x, t[1].y}});
        auto const d2 = sign({p, {t[1].x, t[1].y}, {t[2].x, t[2].y}});
        auto const d3 = sign({p, {t[2].x, t[2].y}, {t[0].x, t[0].y}});

        auto const neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        auto const pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(neg && pos);
    }

    class percent_printer{
    public:
        struct lazy_incer{
            percent_printer& printer;

            ~lazy_incer(){
                ++printer;
            }
        };

        percent_printer(std::size_t const label_width, std::string_view const base_line_label)
            : label_width_(label_width)
        {
            if(base_line_label.size() > label_width_){
                throw std::logic_error("label width is larger then specified");
            }

            fmt::print("{:>{}s}: "
                "===================================================================================================="
                "\n", base_line_label, label_width_);
            std::fflush(stdout);
        }

        void init(std::string_view const label, std::size_t const count){
            if(label.size() > label_width_){
                throw std::logic_error("label width is larger then specified");
            }

            fmt::print("{:>{}s}: ", label, label_width_);
            std::fflush(stdout);

            count_ = count;
            prints_ = 0;
            i_ = 0;
        }

        lazy_incer lazy_inc(){
            return {*this};
        }

    private:
        void operator++(){
            ++i_;

            if(count_ == 0){
                throw std::logic_error("percent printer used without init call");
            }

            if(i_ > count_){
                throw std::logic_error("percent printer run out of range");
            }

            auto const percent =
                static_cast<std::size_t>(std::ceil(static_cast<double>(i_) / static_cast<double>(count_) * 100.));

            bool need_flush = false;
            for(; prints_ < percent; ++prints_){
                fmt::print(">");
                need_flush = true;
            }

            if(i_ == count_){
                fmt::print(" done\n");
                need_flush = true;
            }

            if(need_flush){
                std::fflush(stdout);
            }
        }

        std::size_t label_width_ = 0;
        std::size_t count_ = 0;
        std::size_t prints_ = 0;
        std::size_t i_ = 0;
    };

    template <typename RasterFilter>
    bmp::bitmap<std::vector<raw_pixel<raster_point>>> to_vector_image(
        std::size_t const width,
        std::size_t const height,
        std::vector<raster_point> const& points,
        RasterFilter const& raster_filter
    ){
        auto const range = find_raster_range(points);
        if(range.w() < 2 || range.h() < 2){
            throw std::runtime_error("raster interpolation requires at least 2 columns and 2 rows");
        }

        fmt::print("raster with origin {:d}x{:d} and size {:d}x{:d}\n",
            range.min_x, range.min_y, range.w(), range.h());

        percent_printer progress(30, "base line");

        bmp::bitmap<std::optional<raster_point>> raster_image(range.w(), range.h());
        progress.init("create raster image", points.size());
        for(auto const& p: points){
            auto const printer = progress.lazy_inc();

            auto& target_p = raster_image(range.x(p.rx), range.y(p.ry));
            if(target_p){
                throw std::runtime_error(fmt::format("raster point {:d}x{:d} exists twice", p.rx, p.ry));
            }
            target_p = p;
        }

        progress.init("raster interpolation", (raster_image.h() - 1) * (raster_image.w() - 1));
        bmp::bitmap<std::vector<raw_pixel<raster_point>>> vector_image(width, height);
        for(std::size_t iy = 0; iy < raster_image.h() - 1; ++iy){
            for(std::size_t ix = 0; ix < raster_image.w() - 1; ++ix){
                auto const printer = progress.lazy_inc();

                std::vector<raster_point> region;
                region.reserve(4);

                if(auto const p = raster_image(ix, iy)){
                    region.push_back(*p);
                }

                if(auto const p = raster_image(ix + 1, iy)){
                    region.push_back(*p);
                }

                if(auto const p = raster_image(ix, iy + 1)){
                    region.push_back(*p);
                }

                if(auto const p = raster_image(ix + 1, iy + 1)){
                    region.push_back(*p);
                }

                if(region.size() < 3){
                    continue;
                }

                std::vector<std::array<raster_point, 3>> triangles;
                if(region.size() == 3){
                    triangles.reserve(1);
                    triangles.push_back({region[0], region[1], region[2]});
                }else{
                    triangles.reserve(4);
                    triangles.push_back({region[0], region[1], region[2]});
                    triangles.push_back({region[1], region[2], region[3]});
                    triangles.push_back({region[2], region[3], region[0]});
                    triangles.push_back({region[3], region[0], region[1]});
                }

                for(auto const& t: triangles){
                    // find integer bounting box around the floating point triangle within the target image
                    auto const fx = static_cast<std::size_t>(std::clamp(static_cast<std::int64_t>(std::floor(
                        std::min({t[0].x, t[1].x, t[2].x}))), std::int64_t(0), static_cast<std::int64_t>(width - 1)));
                    auto const tx = static_cast<std::size_t>(std::clamp(static_cast<std::int64_t>(std::ceil(
                        std::max({t[0].x, t[1].x, t[2].x}))), std::int64_t(0), static_cast<std::int64_t>(width - 1)));
                    if(tx == fx){
                        continue;
                    }

                    auto const fy = static_cast<std::size_t>(std::clamp(static_cast<std::int64_t>(std::floor(
                        std::min({t[0].y, t[1].y, t[2].y}))), std::int64_t(0), static_cast<std::int64_t>(height - 1)));
                    auto const ty = static_cast<std::size_t>(std::clamp(static_cast<std::int64_t>(std::ceil(
                        std::max({t[0].y, t[1].y, t[2].y}))), std::int64_t(0), static_cast<std::int64_t>(height - 1)));
                    if(ty == fy){
                        continue;
                    }

                    for(std::size_t y = fy; y <= ty; ++y){
                        for(std::size_t x = fx; x <= tx; ++x){
                            auto const p = bmp::point<double>(static_cast<double>(x), static_cast<double>(y));
                            if(!is_inside(t, p)){
                                continue;
                            }

                            std::array<double, 3> const areas{{
                                area({p, t[1], t[2]}),
                                area({p, t[2], t[0]}),
                                area({p, t[0], t[1]})
                            }};
                            auto const area_sum = areas[0] + areas[1] + areas[2];
                            std::array<double, 3> const weight{{
                                areas[0] / area_sum,
                                areas[1] / area_sum,
                                areas[2] / area_sum
                            }};

                            auto const value =
                                t[0].v * weight[0] +
                                t[1].v * weight[1] +
                                t[2].v * weight[2];

                            auto const index = std::max({
                                std::pair{weight[0], std::size_t(0)},
                                std::pair{weight[1], std::size_t(1)},
                                std::pair{weight[2], std::size_t(2)}}).second;

                            vector_image(x, y).push_back({weight[index], value, t[index].rx, t[index].ry});
                        }
                    }
                }
            }
        }

        if constexpr(!std::same_as<RasterFilter, none_filter>){
            // filter values via raster information
            progress.init("reference filter", vector_image.point_count());
            for(auto& p: vector_image){
                auto const printer = progress.lazy_inc();

                if(p.empty()){
                    continue;
                }

                auto const iter = raster_filter(p);
                std::erase_if(p,
                    [ref_rx = iter->rx, ref_ry = iter->ry](raw_pixel<raster_point> const& v){
                        return std::abs(ref_rx - v.rx) > 1 || std::abs(ref_ry - v.ry) > 1;
                    });

            }
        }

        return vector_image;
    }

    template <typename Point, typename ... RasterFilter>
    bmp::bitmap<double> to_image(
        std::size_t const width,
        std::size_t const height,
        std::vector<Point> const& points,
        RasterFilter const& ... raster_filter
    ){
        using raw_pixel = ply2image::raw_pixel<Point>;

        auto const vector_image = to_vector_image(width, height, points, raster_filter ...);

        bmp::bitmap<double> image(width, height, NaN);
        std::ranges::transform(vector_image, image.begin(),
            [](std::vector<raw_pixel> const& data){
                if(data.empty()){
                    return NaN;
                }else [[likely]]{
                    if(data.size() == 1){
                        return data[0].value;
                    }

                    auto const sum_weight = std::transform_reduce(data.begin(), data.end(), 0., std::plus<double>{},
                        [](raw_pixel const& v){
                            if(v.weight < 0.){
                                throw std::logic_error("negative weight");
                            }
                            return v.weight;
                        });
                    if(sum_weight == 0.){
                        return NaN;
                    }

                    auto const value = std::transform_reduce(data.begin(), data.end(), 0., std::plus<double>{},
                        [](raw_pixel const& v){
                            return v.value * v.weight;
                        });
                    return value / sum_weight;
                }
            });

        return image;
    }


}


void print_help(argparse::ArgumentParser const& program, std::string_view const program_name){
    fmt::print(fmt::emphasis::bold, "{:s}", program_name);
    fmt::print("\n\n"
        "This program converts 3D point clouds in PLY file format to 2D image data in BBF or PNG file format.\n"
        "\n"
        "Links for Group for Quality Assurance and Industrial Image Processing in the Department of Mechanical "
        "Engineering:\n"
        "\n"
        "  - Project page with result examples image\n"
        "      https://gitlab.tu-ilmenau.de/FakMB/QBV/topics/software/ply2image\n"
        "  - PLY file format and how we use it\n"
        "      https://gitlab.tu-ilmenau.de/FakMB/QBV/topics/compendia/project-structure/-/blob/master/doc/doc-3d-file-"
        "formats.md\n"
        "  - BBF file format specification\n"
        "      https://gitlab.tu-ilmenau.de/FakMB/QBV/topics/compendia/bbf-file-format\n"
        "  - PNG file format\n"
        "      https://en.wikipedia.org/wiki/Portable_Network_Graphics\n"
        "\n"
        "For this, two of the PLY properties are interpreted as x and y pixel coordinates for the 2D image. "
        "A third PLY property is interpreted as the value of this pixel.\n"
        "\n"
        "By default, the x, y and z properties of the vertex element are used. This corresponds exactly to the "
        "conversion of a 3D point cloud into a depth map.\n"
        "\n"
        "The values of the properties can be scaled. Before and after scaling, the values can be moved. The shift "
        "before scaling takes place in the unit of the property. The shift after scaling takes place (for x and y) "
        "in 2D pixels. The shift before and after scaling is of course equivalent via the scaling factor. That both "
        "are offered is purely a convenience function.\n"
        "\n"
        "Since the 3D coordinates X and Y are usually not integers, in the 2D image the Z value must be distributed "
        "among the surrounding four 2D pixels. If neighboring 3D X/Y coordinates are further than one unit apart, "
        "then there will be gaps between these pixels in the 2D image. With almost all 3D measurement methods, 2D "
        "neighborhood information of the 3D coordinates can simultaneously be acquired. It is strongly recommended "
        "to always save them with the PLY file and to keep them even in case of global transformations of the 3D "
        "points. If this information is available in x and y direction as a property of the PLY file, it can be used "
        "to perform a dense interpolation between the 2D pixels that were adjacent in 3D. This results in gaps in the "
        "2D image only if the original measurement of the 3D data had also detected a gap. The 2D raster must contain "
        "integer values only. By default it is assumed to be specified in the PLY properties raster_x and raster_y. "
        "If one of these properties is not found in the PLY file, the program prints a warning and performs the "
        "conversion without raster interpolation. Raster interpolation can be switched off explicitly.\n"
        "\n"
        "The raster information can also be used to cleanly separate foreground and background. This is especially "
        "useful for point clouds that have been transformed, as overlaps are very likely to occur. In marginal areas, "
        "however, this may already be the case without transformation. For filtering, the minimum or maximum value is "
        "determined as a reference value in the target pixel. Only values that are adjacent to this reference value "
        "in the raster are included in the target pixel. By default, the minimum is used, which corresponds to a "
        "foreground selection for Z values. (The smaller the value, the closer the pixel was to the acquisition "
        "system)\n"
        "\n"
        "By default, the output image is stored in BBF file format with 64-bit floating point values in the native "
        "byte order of the program's current execution environment. Empty pixels are encoded as NaN (Not a Number). "
        "The BBF specification is linked above. It is a simple raw data format with a 24 bytes header.\n"
        "\n"
        "Saving as PNG is lossy! The output is always a 16 bit grayscale image with alpha channel. The pixel values "
        "range is truncated to 0 to 65535, no overflow or underflow takes place! All pixel values are rounded half up "
        "to integers. Fixed point values can be emulated via the value scaling. For example, to emulate 4 binary "
        "decimal places, the scaling must be set to 16 (=2^4). However, this information is not stored in the image! "
        "So when reading the PNG file later, you have to take care by yourself to interpret the values as fixed-point "
        "numbers again!\n"
        "\n");

    fmt::print("{:s}\n", program.help().str());
}

int main(int argc, char** argv)try{
    using namespace std::literals;
    using namespace ply2image;

    std::locale::global(std::locale("C"));

    argparse::ArgumentParser program(argv[0], "1.1", argparse::default_arguments::version);

    program.add_argument("--help")
        .action([&](std::string const&) {
            print_help(program, argv[0]);
            std::exit(0);
        })
        .default_value(false)
        .help("shows help message and exits")
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-i", "--input")
        .help("3D input file in PLY format")
        .required();

    program.add_argument("-w", "--width")
        .help("width of the output image")
        .scan<'u', std::size_t>()
        .required();
    program.add_argument("-h", "--height")
        .help("height of the output image")
        .scan<'u', std::size_t>()
        .required();

    program.add_argument("-o", "--output")
        .help("name of the output image")
        .required();

    program.add_argument("--output-format")
        .help(fmt::format("file format of the output {:s}", valid_values_string(file_format_strings)))
        .default_value(std::string(file_format_strings[0]));

    program.add_argument("--x-element")
        .help("the PLY element from which the x image positions are taken")
        .default_value("vertex"s);
    program.add_argument("--y-element")
        .help("the PLY element from which the y image positions are taken")
        .default_value("vertex"s);
    program.add_argument("--value-element")
        .help("the PLY element from which the image values are taken")
        .default_value("vertex"s);
    program.add_argument("--x-raster-element")
        .help("the PLY element from which the x raster positions are taken")
        .default_value("vertex"s);
    program.add_argument("--y-raster-element")
        .help("the PLY element from which the y raster positions are taken")
        .default_value("vertex"s);

    program.add_argument("-x", "--x-property")
        .help("the PLY element property used as x image position (must not be a list type)")
        .default_value("x"s);
    program.add_argument("-y", "--y-property")
        .help("the PLY element property used as y image position (must not be a list type)")
        .default_value("y"s);
    program.add_argument("-v", "--value-property")
        .help("the PLY element property converted to image values (must not be a list type)")
        .default_value("z"s);

    program.add_argument("--x-raster-property")
        .help("the PLY element property used as x raster position (must not be a list type)")
        .default_value("raster_x"s);
    program.add_argument("--y-raster-property")
        .help("the PLY element property used as y raster position (must not be a list type)")
        .default_value("raster_y"s);

    program.add_argument("--raster-filter")
        .help(fmt::format("raster filter {:s}", valid_values_string(raster_filter_strings)))
        .default_value(std::string(raster_filter_strings[0]));

    program.add_argument("--disable-raster")
        .help("explicitly disable gap interpolation via raster")
        .implicit_value(true)
        .default_value(false);

    program.add_argument("--x-scale")
        .help("all x values are multiplied by x-scale")
        .scan<'g', double>()
        .default_value(1.);
    program.add_argument("--y-scale")
        .help("all y values are multiplied by y-scale")
        .scan<'g', double>()
        .default_value(1.);
    program.add_argument("--value-scale")
        .help("all pixel values are multiplied by value-scale")
        .scan<'g', double>()
        .default_value(1.);

    program.add_argument("--x-pre-scale-offset")
        .help("all x values are added with x-pre-scale-offset before scaling")
        .scan<'g', double>()
        .default_value(0.);
    program.add_argument("--y-pre-scale-offset")
        .help("all y values are added with y-pre-scale-offset before scaling")
        .scan<'g', double>()
        .default_value(0.);
    program.add_argument("--value-pre-scale-offset")
        .help("all pixel values are added with value-pre-scale-offset before scaling")
        .scan<'g', double>()
        .default_value(0.);

    program.add_argument("--x-post-scale-offset")
        .help("all x values are added with x-post-scale-offset after scaling")
        .scan<'g', double>()
        .default_value(0.);
    program.add_argument("--y-post-scale-offset")
        .help("all y values are added with y-post-scale-offset after scaling")
        .scan<'g', double>()
        .default_value(0.);
    program.add_argument("--value-post-scale-offset")
        .help("all pixel values are added with value-post-scale-offset after scaling")
        .scan<'g', double>()
        .default_value(0.);

    try{
        program.parse_args(argc, argv);
    }catch(std::runtime_error const& error){
        fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "Error: {:s}\n\n", error.what());
        print_help(program, argv[0]);
        return -1;
    }

    auto const input_filepath = std::filesystem::path(program.get<std::string>("-i"));
    auto const output_filepath = std::filesystem::path(program.get<std::string>("-o"));
    auto const output_format =
        parse_enum_string<file_format>(file_format_strings, program.get<std::string>("--output-format"));

    if(auto const
        ext = trim_left(output_filepath.extension().string(), '.'),
        format = std::string(file_format_strings[static_cast<int>(output_format)]);
        ext != format
    ){
        throw std::runtime_error(fmt::format(
            "file extension of output file {:s} is different from specified output format {:s}",
            std::quoted(ext), std::quoted(format)));
    }

    auto const width = program.get<std::size_t>("-w");
    auto const height = program.get<std::size_t>("-h");

    auto const x_element = program.get<std::string>("--x-element");
    auto const y_element = program.get<std::string>("--y-element");
    auto const v_element = program.get<std::string>("--value-element");

    auto const x_property = program.get<std::string>("-x");
    auto const y_property = program.get<std::string>("-y");
    auto const v_property = program.get<std::string>("-v");

    auto const arg_xr_element = get_raster<std::string>(program, "--x-raster-element", "--disable-raster");
    auto const arg_yr_element = get_raster<std::string>(program, "--y-raster-element", "--disable-raster");
    auto const arg_xr_property = get_raster<std::string>(program, "--x-raster-property", "--disable-raster");
    auto const arg_yr_property = get_raster<std::string>(program, "--y-raster-property", "--disable-raster");
    auto const explicit_raster =
        program.is_used("--x-raster-element") ||
        program.is_used("--y-raster-element") ||
        program.is_used("--x-raster-property") ||
        program.is_used("--y-raster-property");
    auto const filter =
        parse_enum_string<raster_filter>(raster_filter_strings, program.get<std::string>("--raster-filter"));

    auto const x_scale = program.get<double>("--x-scale");
    auto const y_scale = program.get<double>("--y-scale");
    auto const v_scale = program.get<double>("--value-scale");

    auto const x_pre_scale = program.get<double>("--x-pre-scale-offset");
    auto const y_pre_scale = program.get<double>("--y-pre-scale-offset");
    auto const v_pre_scale = program.get<double>("--value-pre-scale-offset");

    auto const x_post_scale = program.get<double>("--x-post-scale-offset");
    auto const y_post_scale = program.get<double>("--y-post-scale-offset");
    auto const v_post_scale = program.get<double>("--value-post-scale-offset");

    // load file
    ply::ply data;
    data.load(input_filepath);

    {
        auto names = data.element_names();
        std::ranges::sort(names);
        if(std::unique(names.begin(), names.end()) != names.end()){
            fmt::print(fmt::emphasis::bold | fg(fmt::color::orange),
                "Warning: PLY file contains duplicate element names, when accessed the first element is used\n");
        }
    }

    // display file structure
    {
        auto used_properties = std::vector<std::array<std::string_view, 2>>{
            {x_element, x_property}, {y_element, y_property}, {v_element, v_property}};

        if(arg_xr_element){
            used_properties.push_back({*arg_xr_element, *arg_xr_property});
        }

        if(arg_yr_element){
            used_properties.push_back({*arg_yr_element, *arg_yr_property});
        }

        auto const element_count = data.element_count();
        auto const element_count_width = fmt::format("{:d}", element_count).size();
        for(std::size_t i = 0; i < element_count; ++i){
            auto const element_name = data.element_name(i);
            fmt::print("element {:>{}d} {:s} with {:d} values\n", i, element_count_width,
                std::quoted(element_name), data.value_count(i));

            {
                auto names = data.property_names(i);
                std::ranges::sort(names);
                if(std::unique(names.begin(), names.end()) != names.end()){
                    fmt::print(fmt::emphasis::bold | fg(fmt::color::orange),
                        "    Warning: Element {:s} contains duplicate property names, "
                        "when accessed the first property is used\n", element_name);
                }
            }

            auto const property_count = data.property_count(i);
            auto const property_count_width = fmt::format("{:d}", property_count).size();
            for(std::size_t j = 0; j < property_count; ++j){
                auto const property_name = data.property_name(i, j);
                auto const test = std::array{element_name, property_name};
                bool const used = contains(used_properties, test);
                if(used){
                    std::erase(used_properties, test);
                }
                fmt::print(used ? fmt::emphasis::bold : fmt::emphasis(),
                    "    property {:>{}d} {:s} with type {:s}\n", j, property_count_width,
                    std::quoted(property_name), data.property_type_name(i, j));
            }
        }
    }

    auto const [xr_element, xr_property, yr_element, yr_property] =
        [&]()->std::array<std::optional<std::string>, 4>{
            if(arg_xr_element){
                if(explicit_raster || (
                    data.contains_property(*arg_xr_element, *arg_xr_property) &&
                    data.contains_property(*arg_yr_element, *arg_yr_property)
                )){
                    return {arg_xr_element, arg_xr_property, arg_yr_element, arg_yr_property};
                }else{
                    fmt::print(fmt::emphasis::bold | fg(fmt::color::orange),
                        "Warning: Disable raster interpulation because element vertex does not contain the "
                        "properties raster_x and raster_y. Use --disable-raster to disable this warning.\n");
                }
            }

            return {std::nullopt, std::nullopt, std::nullopt, std::nullopt};
        }();


    // value count of the three used properties
    auto const count = [&]{
        auto const x_count = data.value_count(x_element);

        if(x_count != data.value_count(y_element)){
            throw std::runtime_error("--y-element has different value count then --x-element");
        }

        if(x_count != data.value_count(v_element)){
            throw std::runtime_error("--v-element has different value count then --x-element");
        }

        if(xr_element && x_count != data.value_count(*xr_element)){
            throw std::runtime_error("--x-raster-element has different value count then --x-element");
        }

        if(yr_element && x_count != data.value_count(*yr_element)){
            throw std::runtime_error("--y-raster-element has different value count then --x-element");
        }

        return x_count;
    }();

    if(count == 0){
        throw std::runtime_error("value count is 0");
    }

    auto const image_convert =
        [&]<typename Point>(std::type_identity<Point>, auto const& ... raster_filter){
            // extract used data
            std::vector<Point> points(count);
            auto const convert = [&points]<ply::valid_value T>(auto const& setter, std::span<T const> const list){
                if constexpr(ply::scalar_value<T>)[[likely]]{
                    for(std::size_t i = 0; i < points.size(); ++i){
                        setter(points[i], list[i]);
                    }
                }else{
                    throw std::runtime_error("list type properties are not supported");
                }
            };

            auto const set_x = [=](Point& p, auto const v){
                    p.x = (static_cast<double>(v) + x_pre_scale) * x_scale + x_post_scale;
                };
            auto const set_y = [=](Point& p, auto const v){
                    p.y = (static_cast<double>(v) + y_pre_scale) * y_scale + y_post_scale;
                };
            auto const set_v = [=](Point& p, auto const v){
                    p.v = (static_cast<double>(v) + v_pre_scale) * v_scale + v_post_scale;
                };
            std::visit([=](auto const& v){ convert(set_x, v); }, data.values(x_element, x_property));
            std::visit([=](auto const& v){ convert(set_y, v); }, data.values(y_element, y_property));
            std::visit([=](auto const& v){ convert(set_v, v); }, data.values(v_element, v_property));

            if constexpr(std::is_same_v<Point, raster_point>){
                auto const set_rx = [](Point& p, auto const v){ p.rx = raster_convert(v); };
                auto const set_ry = [](Point& p, auto const v){ p.ry = raster_convert(v); };
                std::visit([=](auto const& v){ convert(set_rx, v); }, data.values(*xr_element, *xr_property));
                std::visit([=](auto const& v){ convert(set_ry, v); }, data.values(*yr_element, *yr_property));
            }

            // convert list to image
            return to_image<Point>(width, height, points, raster_filter ...);
        };

    auto const image =
        [&]{
            if(xr_element){
                switch(filter){
                    case raster_filter::min:
                        return image_convert(std::type_identity<raster_point>(), min_value_filter{});
                    case raster_filter::max:
                        return image_convert(std::type_identity<raster_point>(), max_value_filter{});
                    case raster_filter::none:
                        return image_convert(std::type_identity<raster_point>(), none_filter{});
                }
                throw std::logic_error("invalid raster filter");
            }else{
                return image_convert(std::type_identity<point>());
            }
        }();

    [&]{
        switch(output_format){
            case file_format::bbf: {
                bmp::binary_write(image, output_filepath.string());
            } return;
            case file_format::png: {
                bmp::bitmap<bmp::pixel::masked_g16u> png_image(image.size());
                std::ranges::transform(image, png_image.begin(),
                    [](double const v){
                        if(std::isnan(v)){
                            return bmp::pixel::masked_g16u{.v = {}, .m = true};
                        }else{
                            return bmp::pixel::masked_g16u{
                                .v = static_cast<std::uint16_t>(std::round(std::clamp(v, 0., 65535.))), .m = false};
                        }
                    });

                bmp::png::write(png_image, output_filepath.string());
            } return;
        }

        throw std::logic_error("invalid file format");
    }();
}catch(std::system_error const& error){
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red),
        "System error:\n"
        "  Category: {:s}\n"
        "      Code: {:d}\n"
        "   Message: {:s}\n",
        error.code().category().name(), error.code().value(), error.code().message());
    return 3;
}catch(std::exception const& error){
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "Error: {:s}\n", error.what());
    return 2;
}
