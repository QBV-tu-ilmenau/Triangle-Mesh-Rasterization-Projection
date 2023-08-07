#pragma once

#include "text.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <boost/endian.hpp>

#include <charconv>
#include <cmath>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <span>
#include <variant>


namespace ply{


    using namespace std::literals;

    template<typename, template<typename ...> typename>
    inline constexpr bool is_instanc_of = false;

    template<template<typename ...> typename T, typename... Args>
    inline constexpr bool is_instanc_of<T<Args ...>, T> = true;

    template<typename T, typename ... U>
    concept any_of = (std::same_as<T, U> || ...);

    template <typename T>
    concept scalar_value =
        any_of<T, std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, std::int32_t, std::uint32_t, float, double>;

    template <typename T>
    concept list_value = is_instanc_of<T, std::vector> && scalar_value<typename T::value_type>;

    template <typename T>
    concept valid_value = scalar_value<T> || list_value<T>;


    constexpr std::size_t find_index(std::ranges::sized_range auto container, std::string_view element)noexcept{
        return std::size_t(std::ranges::find(container, element) - std::begin(container));
    }

    template <std::size_t AS, char const* A, std::size_t BS, char const*B>
    consteval std::string_view join(){
        return []<std::size_t ... I>(std::index_sequence<I ...>){
            constexpr std::array<char, AS + BS> memory{{(I < AS ? A[I] : B[I - AS]) ...}};
            return std::string_view(memory.begin(), memory.end());
        }(std::make_index_sequence<AS + BS>());
    }


    static constexpr std::array file_type_strings
        {"ascii"sv, "binary_big_endian"sv, "binary_little_endian"sv};

    enum class file_type: std::size_t{
        ascii = 0,
        binary_big_endian = 1,
        binary_little_endian = 2
    };

    static constexpr std::array header_entry_strings
        {"comment"sv, "property"sv, "element"sv, "end_header"sv};

    static constexpr std::array type_strings
        {"char"sv, "uchar"sv, "short"sv, "ushort"sv, "int"sv, "uint"sv, "float"sv, "double"sv};

    static constexpr std::array list_type_strings
        {"list of char"sv, "list of uchar"sv, "list of short"sv, "list of ushort"sv,
        "list of int"sv, "list of uint"sv, "list of float"sv, "list of double"sv};

    template <scalar_value T> static std::size_t index_of;
    template <> constexpr std::size_t index_of<  std::int8_t> = 0;
    template <> constexpr std::size_t index_of< std::uint8_t> = 1;
    template <> constexpr std::size_t index_of< std::int16_t> = 2;
    template <> constexpr std::size_t index_of<std::uint16_t> = 3;
    template <> constexpr std::size_t index_of< std::int32_t> = 4;
    template <> constexpr std::size_t index_of<std::uint32_t> = 5;
    template <> constexpr std::size_t index_of<        float> = 6;
    template <> constexpr std::size_t index_of<       double> = 7;

    template <valid_value T>
    constexpr std::string_view as_string(){
        if constexpr(list_value<T>){
            return list_type_strings[index_of<typename T::value_type>];
        }else{
            return type_strings[index_of<T>];
        }
    }

    template <typename T>
    requires(scalar_value<T> || std::same_as<T, std::size_t>)
    constexpr T parse_value(std::string_view const text){
        constexpr auto from_chars_has_supports_T = requires(char const* f, char const* t, T& v){
            std::from_chars(f, t, v);
        };

        if constexpr(!std::is_floating_point_v<T> || from_chars_has_supports_T){
            T value;
            auto [ptr, error_code] = std::from_chars(text.data(), text.data() + text.size(), value);
            if(error_code == std::errc()){
                return value;
            }
        }else if constexpr(std::is_same_v<T, float>){
            return std::stof(std::string(text));
        }else if constexpr(std::is_same_v<T, double>){
            return std::stod(std::string(text));
        }

        constexpr auto type_string =
            []{
                if constexpr(scalar_value<T>){
                    return type_strings[index_of<T>];
                }else{
                    return "std::size_t"sv;
                }
            }();
        throw std::runtime_error(fmt::format("Can not convert {:s} to {:s}", std::quoted(text), type_string));
    }

    template <scalar_value T>
    T load_ascii(std::string_view& line){
        auto const [value, rest] = detail::split_front(line);
        line = rest;
        return parse_value<T>(value);
    }

    template <scalar_value T, boost::endian::order Order>
    T load_binary(std::istream& is){
        std::array<unsigned char, sizeof(T)> buffer;
        is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        return boost::endian::endian_load<T, sizeof(T), Order>(buffer.data());
    }

    template <scalar_value T>
    T load_big_endian(std::istream& is){
        return load_binary<T, boost::endian::order::big>(is);
    }

    template <scalar_value T>
    T load_little_endian(std::istream& is){
        return load_binary<T, boost::endian::order::little>(is);
    }

    template <scalar_value T>
    struct list_count_loader{
        static constexpr std::size_t convert_value(T const count){
            if constexpr(std::is_floating_point_v<T>){
                if(count != std::floor(count)){
                    throw std::runtime_error("list property length is not integer");
                }else if(count < 0){
                    throw std::runtime_error("list property length is negative");
                }
                return static_cast<std::size_t>(std::floor(count));
            }else if constexpr(std::is_signed_v<T>){
                if(count < 0){
                    throw std::runtime_error("list property length is negative");
                }
                return static_cast<std::size_t>(count);
            }else{
                return count;
            }
        }

        static constexpr std::size_t load_ascii(std::string_view& line){
            return convert_value(::ply::load_ascii<T>(line));
        }

        static std::size_t load_big_endian(std::istream& is){
            return convert_value(::ply::load_big_endian<T>(is));
        }

        static std::size_t load_little_endian(std::istream& is){
            return convert_value(::ply::load_little_endian<T>(is));
        }
    };

    using list_count_loader_variant = std::variant<
        list_count_loader<std::int8_t>,
        list_count_loader<std::uint8_t>,
        list_count_loader<std::int16_t>,
        list_count_loader<std::uint16_t>,
        list_count_loader<std::int32_t>,
        list_count_loader<std::uint32_t>,
        list_count_loader<float>,
        list_count_loader<double>>;

    static constexpr std::array<list_count_loader_variant(*)(), 8> make_list_count_loader = {
        []()->list_count_loader_variant{ return list_count_loader<std::int8_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<std::uint8_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<std::int16_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<std::uint16_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<std::int32_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<std::uint32_t>{}; },
        []()->list_count_loader_variant{ return list_count_loader<float>{}; },
        []()->list_count_loader_variant{ return list_count_loader<double>{}; }};

    template <scalar_value> class scalar_property;
    template <scalar_value> class list_property;

    using property_variant = std::variant<
        scalar_property<std::int8_t>,
        scalar_property<std::uint8_t>,
        scalar_property<std::int16_t>,
        scalar_property<std::uint16_t>,
        scalar_property<std::int32_t>,
        scalar_property<std::uint32_t>,
        scalar_property<float>,
        scalar_property<double>,
        list_property<std::int8_t>,
        list_property<std::uint8_t>,
        list_property<std::int16_t>,
        list_property<std::uint16_t>,
        list_property<std::int32_t>,
        list_property<std::uint32_t>,
        list_property<float>,
        list_property<double>>;

    template <valid_value T>
    class base_property{
    public:
        base_property(std::string name, std::size_t const count)
            : name_(std::move(name))
            , values_(std::make_unique<T[]>(count))
            {}

        std::string_view name()const noexcept{
            return name_;
        }

        std::span<T const> values(std::size_t const count)const noexcept{
            return {values_.get(), values_.get() + count};
        }

    protected:
        std::string name_;
        std::unique_ptr<T[]> values_;
    };


    template <scalar_value T>
    class scalar_property: public base_property<T>{
    public:
        scalar_property(std::string name, std::size_t const count)
            : base_property<T>(std::move(name), count)
            {}

        void load_ascii(std::size_t const i, std::string_view& line){
            this->values_[i] = ::ply::load_ascii<T>(line);
        }

        void load_big_endian(std::size_t const i, std::istream& is){
            this->values_[i] = ::ply::load_big_endian<T>(is);
        }

        void load_little_endian(std::size_t const i, std::istream& is){
            this->values_[i] = ::ply::load_little_endian<T>(is);
        }
    };

    template <scalar_value T>
    class list_property: public base_property<std::vector<T>>{
    public:
        list_property(std::string name, std::size_t const count, list_count_loader_variant const loader)
            : base_property<std::vector<T>>(std::move(name), count)
            , loader_(loader)
            {}

        void load_ascii(std::size_t const i, std::string_view& line){
            auto const count = std::visit([&line](auto const& loader){
                return loader.load_ascii(line);
            }, loader_);
            this->values_[i].reserve(count);
            for(std::size_t j = 0; j < count; ++j){
                this->values_[i].push_back(::ply::load_ascii<T>(line));
            }
        }

        void load_big_endian(std::size_t const i, std::istream& is){
            auto const count = std::visit([&is](auto const& loader){
                return loader.load_big_endian(is);
            }, loader_);
            this->values_[i].reserve(count);
            for(std::size_t j = 0; j < count; ++j){
                this->values_[i].push_back(::ply::load_big_endian<T>(is));
            }
        }

        void load_little_endian(std::size_t const i, std::istream& is){
            auto const count = std::visit([&is](auto const& loader){
                return loader.load_little_endian(is);
            }, loader_);
            this->values_[i].reserve(count);
            for(std::size_t j = 0; j < count; ++j){
                this->values_[i].push_back(::ply::load_little_endian<T>(is));
            }
        }

    private:
        list_count_loader_variant loader_;
    };


    std::string read_line(std::istream& is, std::size_t& line_number){
        ++line_number;

        std::string line;
        std::getline(is, line);
        return line;
    }

    static constexpr std::array<property_variant(*)(std::string, std::size_t), 8>
        make_scalar_property = {
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::int8_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::uint8_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::int16_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::uint16_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::int32_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<std::uint32_t>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<float>(std::move(name), count); },
            [](std::string name, std::size_t const count)
                ->property_variant{ return scalar_property<double>(std::move(name), count); }};

    static constexpr std::array<property_variant(*)(std::string, std::size_t, list_count_loader_variant), 8>
        make_list_property = {
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::int8_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::uint8_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::int16_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::uint16_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::int32_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<std::uint32_t>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<float>(std::move(name), count, loader); },
            [](std::string name, std::size_t const count, list_count_loader_variant const loader)
                ->property_variant{ return list_property<double>(std::move(name), count, loader); }};


    using value_variant = std::variant<
        std::span<std::int8_t const>,
        std::span<std::uint8_t const>,
        std::span<std::int16_t const>,
        std::span<std::uint16_t const>,
        std::span<std::int32_t const>,
        std::span<std::uint32_t const>,
        std::span<float const>,
        std::span<double const>,
        std::span<std::vector<std::int8_t> const>,
        std::span<std::vector<std::uint8_t> const>,
        std::span<std::vector<std::int16_t> const>,
        std::span<std::vector<std::uint16_t> const>,
        std::span<std::vector<std::int32_t> const>,
        std::span<std::vector<std::uint32_t> const>,
        std::span<std::vector<float> const>,
        std::span<std::vector<double> const>>;


    class element{
    public:
        element(std::string name, std::size_t const count)
            : name_(std::move(name))
            , count_(count)
            {}

        void add_property(std::string_view specification){
            auto const [type, rest] = detail::split_front(specification);
            if(type == "list"sv){
                auto const [count_data, count_rest] = detail::split_front(rest);
                auto const count_index = find_index(type_strings, count_data);
                if(count_index > type_strings.size()){
                    throw std::runtime_error("invalid property list count type");
                }

                auto const [type_data, property_name] = detail::split_front(count_rest);
                auto const data_index = find_index(type_strings, type_data);
                if(data_index > type_strings.size()){
                    throw std::runtime_error("invalid property list data type");
                }
                if(property_name.empty()){
                    throw std::runtime_error("no list property name defined");
                }

                auto const count_loader = make_list_count_loader[count_index]();
                properties_.push_back(make_list_property[data_index](std::string(property_name), count_, count_loader));
            }else{
                auto const data_index = find_index(type_strings, type);
                if(data_index > type_strings.size()){
                    throw std::runtime_error("invalid property data type");
                }
                if(rest.empty()){
                    throw std::runtime_error("no property name defined");
                }

                properties_.push_back(make_scalar_property[data_index](std::string(rest), count_));
            }
        }

        void load_ascii(std::istream& is, std::size_t& line_number){
            for(std::size_t i = 0; i < count_; ++i){
                auto const line = read_line(is, line_number);
                auto trimmed_line = detail::trim(line);

                for(auto& property: properties_){
                    std::visit([i, &trimmed_line](auto& property){
                        property.load_ascii(i, trimmed_line);
                    }, property);
                }

                if(!trimmed_line.empty()){
                    throw std::runtime_error("data line contains more values than specified");
                }
            }
        }

        void load_big_endian(std::istream& is){
            for(std::size_t i = 0; i < count_; ++i){
                for(auto& property: properties_){
                    std::visit([i, &is](auto& property){
                        property.load_big_endian(i, is);
                    }, property);
                }
            }
        }

        void load_little_endian(std::istream& is){
            for(std::size_t i = 0; i < count_; ++i){
                for(auto& property: properties_){
                    std::visit([i, &is](auto& property){
                        property.load_little_endian(i, is);
                    }, property);
                }
            }
        }

        std::string_view name()const noexcept{
            return name_;
        }

        std::size_t property_count()const noexcept{
            return properties_.size();
        }

        std::vector<std::string_view> property_names()const{
            std::vector<std::string_view> names;
            names.reserve(property_count());
            std::ranges::transform(properties_, std::back_inserter(names), [](auto const& property){
                return std::visit([](auto const& property){ return property.name(); }, property);
            });
            return names;
        }

        std::size_t property_index(std::string_view const name)const{
            auto const iter = find_property(name);
            if(iter == properties_.end()){
                throw std::runtime_error(fmt::format(
                    "PLY element {:s} contains no property {:s}", std::quoted(name_), std::quoted(name)));
            }
            return std::size_t(iter - properties_.begin());
        }

        std::string_view property_name(std::size_t const index)const{
            return std::visit([](auto const& property){ return property.name(); }, properties_[index]);
        }

        std::string_view property_type_name(std::size_t const index)const{
            return std::visit([]<typename T>(base_property<T> const&){ return as_string<T>(); }, properties_[index]);
        }

        bool contains(std::string_view const name)const noexcept{
            return find_property(name) != properties_.end();
        }

        std::size_t value_count()const noexcept{
            return count_;
        }

        value_variant values(std::size_t const index)const noexcept{
            return std::visit(
                [this](auto const& property)->value_variant{
                    return property.values(count_);
                }, properties_[index]);
        }

        template <valid_value T>
        value_variant values(std::size_t const index)const{
            auto const values = std::get_if<T>(properties_[index]);
            if(values == nullptr){
                throw std::runtime_error(fmt::format(
                    "PLY element {:s} property {:s} accessed as {:s} but its type is {:s}",
                    std::quoted(name_), std::quoted(property_name(index)), as_string<T>(),
                    property_type_name(index)));
            }
        }


    private:
        std::vector<property_variant>::const_iterator find_property(std::string_view const name)const{
            return std::ranges::find_if(properties_, [=](auto const& property){
                return std::visit([=](auto const& property){ return property.name() == name; }, property);
            });
        }

        std::string name_;
        std::size_t count_;
        std::vector<property_variant> properties_;
    };

    class ply{
    public:
        void load(std::filesystem::path const& filepath){
            if(!std::filesystem::exists(filepath)){
                throw std::runtime_error("file does not exist");
            }

            std::ifstream is(filepath, std::ios::binary);
            load(is);
        }

        void load(std::istream& is){
            comments_.clear();
            elements_.clear();

            std::size_t line_number = 0;

            try{
                is.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);

                if(detail::trim(read_line(is, line_number)) != "ply"sv){
                    throw std::runtime_error("invalid first line");
                }

                process_header(is, line_number);
            }catch(std::runtime_error const& error){
                rethrow_with_line(is, error, line_number);
            }

            process_data(is, line_number);
        }


        std::size_t comment_count()const noexcept{
            return comments_.size();
        }

        std::vector<std::string> const& comments()noexcept{
            return comments_;
        }

        std::string const& comment(std::size_t const index)const noexcept{
            return comments_[index];
        }


        std::size_t element_count()const noexcept{
            return elements_.size();
        }

        std::vector<std::string_view> element_names()const{
            std::vector<std::string_view> names;
            names.reserve(elements_.size());
            std::ranges::transform(elements_, std::back_inserter(names), [](auto const& element){
                return element.name();
            });
            return names;
        }

        std::size_t element_index(std::string_view const name)const{
            auto const iter = find_element(name);
            if(iter == elements_.end()){
                throw std::runtime_error(fmt::format("PLY contains no element {:s}", std::quoted(name)));
            }
            return std::size_t(iter - elements_.begin());
        }

        std::string_view element_name(std::size_t const index)const noexcept{
            return elements_[index].name();
        }

        bool contains_element(std::string_view const name)const noexcept{
            return find_element(name) != elements_.end();
        }


        std::size_t value_count(std::size_t const element_index)const noexcept{
            return elements_[element_index].value_count();
        }

        std::size_t value_count(std::string_view const element_name)const{
            return value_count(element_index(element_name));
        }


        std::size_t property_count(std::size_t const element_index)const{
            return elements_[element_index].property_count();
        }

        std::size_t property_count(std::string_view const element_name)const{
            return property_count(element_index(element_name));
        }

        std::vector<std::string_view> property_names(std::size_t const element_index)const{
            return elements_[element_index].property_names();
        }

        std::vector<std::string_view> property_names(std::string_view const element_name)const{
            return property_names(element_index(element_name));
        }

        std::size_t property_index(std::size_t const element_index, std::string_view const property_name)const{
            return elements_[element_index].property_index(property_name);
        }

        std::size_t property_index(std::string_view const element_name, std::string_view const property_name)const{
            return property_index(element_index(element_name), property_name);
        }

        std::string_view property_name(std::size_t const element_index, std::size_t const property_index)const{
            return elements_[element_index].property_name(property_index);
        }

        std::string_view property_name(std::string_view const element_name, std::size_t const property_index)const{
            return property_name(element_index(element_name), property_index);
        }

        bool contains_property(std::size_t const element_index, std::string_view const property_name)const noexcept{
            return elements_[element_index].contains(property_name);
        }

        bool contains_property(std::string_view const element_name, std::string_view const property_name)const noexcept{
            auto const iter = find_element(element_name);
            if(iter == elements_.end()){
                return false;
            }
            return iter->contains(property_name);
        }


        std::string_view property_type_name(
            std::size_t const element_index,
            std::size_t const property_index
        )const noexcept{
            return elements_[element_index].property_type_name(property_index);
        }

        std::string_view property_type_name(
            std::size_t const element_index,
            std::string_view const property_name
        )const noexcept{
            return property_type_name(element_index, property_index(element_index, property_name));
        }

        std::string_view property_type_name(
            std::string_view const element_name,
            std::size_t const property_index
        )const noexcept{
            return property_type_name(element_index(element_name), property_index);
        }

        std::string_view property_type_name(
            std::string_view const element_name,
            std::string_view const property_name
        )const noexcept{
            return property_type_name(element_index(element_name), property_name);
        }


        value_variant values(std::size_t const element_index, std::size_t const property_index)const{
            return elements_[element_index].values(property_index);
        }

        value_variant values(std::size_t const element_index, std::string_view const property_name)const{
            return values(element_index, property_index(element_index, property_name));
        }

        value_variant values(std::string_view const element_name, std::size_t const property_index)const{
            return values(element_index(element_name), property_index);
        }

        value_variant values(std::string_view const element_name, std::string_view const property_name)const{
            return values(element_index(element_name), property_name);
        }


        template <valid_value T>
        std::span<T const> values(std::size_t const element_index, std::size_t const property_index)const{
            return elements_[element_index].values<T>(property_index);
        }

        template <valid_value T>
        std::span<T const> values(std::size_t const element_index, std::string_view const property_name)const{
            return values<T>(element_index, property_index(element_index, property_name));
        }

        template <valid_value T>
        std::span<T const> values(std::string_view const element_name, std::size_t const property_index)const{
            return values<T>(element_index(element_name), property_index);
        }

        template <valid_value T>
        std::span<T const> values(std::string_view const element_name, std::string_view const property_name)const{
            return values<T>(element_index(element_name), property_name);
        }


// TODO: conversion access, note this can not convert between scalar and list types!
//         template <valid_value T>
//         auto values_as(std::size_t const element_index, std::size_t const property_index)const{
//             return std::visit([](auto const span){
//                 return std::views::all(span) | std::views::transform([](auto const& value){
//                     if constexpr(list_value<T>){
//                         return std::views::all(value) | std::views::transform([](auto const& value){
//                             return static_cast<T>(value);
//                         });
//                     }else{
//                         return static_cast<T>(value);
//                     }
//                 });
//             }, values(element_index, property_index));
//         }
//
//         template <valid_value T>
//         auto values_as(std::size_t const element_index, std::string_view const property_name)const{
//             return values_as<T>(element_index, property_index(element_index, property_name));
//         }
//
//         template <valid_value T>
//         auto values_as(std::string_view const element_name, std::size_t const property_index)const{
//             return values_as<T>(element_index(element_name), property_index);
//         }
//
//         template <valid_value T>
//         auto values_as(std::string_view const element_name, std::string_view const property_name)const{
//             return values_as<T>(element_index(element_name), property_name);
//         }


    private:
        std::vector<element>::const_iterator find_element(std::string_view const element_name)const{
            return std::ranges::find_if(elements_, [=](auto const& element){ return element.name() == element_name; });
        }

        [[noreturn]] void rethrow_with_line(
            std::istream& is,
            std::runtime_error const& error,
            std::size_t const line_number
        ){
            if(auto const* system_error = dynamic_cast<std::system_error const*>(&error)){
                if(is.eof()){
                    throw std::runtime_error(fmt::format("line {:d}: unexpected end of input", line_number));
                }else{
                    throw *system_error;
                }
            }
            throw std::runtime_error(fmt::format("line {:d}: {:s}", line_number, error.what()));
        }

        [[noreturn]] void rethrow_while_binary_read(std::istream& is, std::runtime_error const& error){
            if(auto const* system_error = dynamic_cast<std::system_error const*>(&error)){
                if(is.eof()){
                    throw std::runtime_error("binary file part: unexpected end of input");
                }else{
                    throw *system_error;
                }
            }
            throw std::runtime_error(fmt::format("binary file part: {:s}", error.what()));
        }

        void process_header(std::istream& is, std::size_t& line_number){
            file_type_ = [&]{
                auto const line = read_line(is, line_number);
                auto const [entry, type, version] = detail::split_front<3>(line);
                if(entry != "format"sv){
                    throw std::runtime_error("invalid format line");
                }

                if(version != "1.0"sv){
                    throw std::runtime_error("unsupported format version");
                }

                auto const format_index = find_index(file_type_strings, type);
                if(format_index >= file_type_strings.size()){
                    throw std::runtime_error("invalid format");
                }

                return file_type(format_index);
            }();

            for(;;){
                auto const line = read_line(is, line_number);
                auto const [entry, spec] = detail::split_front(detail::trim(line));
                auto const entry_index = find_index(header_entry_strings, entry);
                if(entry_index >= header_entry_strings.size()){
                    throw std::runtime_error("invalid format");
                }

                switch(entry_index){
                    case find_index(header_entry_strings, "comment"sv):
                        comments_.emplace_back(spec);
                    continue;
                    case find_index(header_entry_strings, "element"sv):
                        add_element(spec);
                    continue;
                    case find_index(header_entry_strings, "property"sv):
                        add_property(spec);
                    continue;
                    case find_index(header_entry_strings, "end_header"sv):
                        break;
                }

                break;
            }
        }

        void add_element(std::string_view specification){
            auto const [name, count] = detail::split_back(specification);
            elements_.emplace_back(std::string(name), parse_value<std::size_t>(count));
        }

        void add_property(std::string_view specification){
            if(elements_.empty()){
                throw std::runtime_error("property without previous element");
            }

            elements_.back().add_property(specification);
        }

        void process_data(std::istream& is, std::size_t& line_number){
            switch(file_type_){
                using enum file_type;
                case ascii:
                    try{
                        for(auto& element: elements_){
                            element.load_ascii(is, line_number);
                        }
                    }catch(std::runtime_error const& error){
                        rethrow_with_line(is, error, line_number);
                    }

                    return;
                case binary_big_endian:
                    try{
                        for(auto& element: elements_){
                            element.load_big_endian(is);
                        }
                        return;
                    }catch(std::runtime_error const& error){
                        rethrow_while_binary_read(is, error);
                    }
                case binary_little_endian:
                    try{
                        for(auto& element: elements_){
                            element.load_little_endian(is);
                        }
                    }catch(std::runtime_error const& error){
                        rethrow_while_binary_read(is, error);
                    }
                    return;
            }

            throw std::logic_error("invalid PLY file type");
        }

        std::vector<std::string> comments_;
        std::vector<element> elements_;
        file_type file_type_;
    };


}
