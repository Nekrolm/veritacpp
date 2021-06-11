#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <tuple>
#include <utility>

namespace veritacpp::utils {

namespace detail {    
template <class T>
struct IsTuple : std::false_type {};

template <class... T>
struct IsTuple<std::tuple<T...>> : std::true_type {};
}


template <class T>
concept Tuple = detail::IsTuple<T>::value;

/**
 * Split tuple into pair:
 * [0...split_pos), [split_pos...tuple_size)
 */
template <uint64_t split_pos, Tuple T>
constexpr auto split(T t) {
    constexpr auto t_size = std::tuple_size_v<T>;
    if constexpr (split_pos == 0) {
        return make_pair(
            std::tuple<>{},
            t
        );
    }
    if constexpr (split_pos >= t_size) {
        return std::make_pair(t, std::tuple<>{});
    } else { // else ommiting breaks conditional compilation!
        auto get_right = [&]<uint64_t... idx>(std::integer_sequence<uint64_t, idx...>) {
            return std::make_tuple(std::get<split_pos + idx>(t)...); 
        };
        auto get_left = [&]<uint64_t... idx>(std::integer_sequence<uint64_t, idx...>) {
            return std::make_tuple(std::get<idx>(t)...); 
        };
        constexpr auto delta = t_size - split_pos;
    
        return std::make_pair(
            get_left(std::make_index_sequence<split_pos>{}),
            get_right(std::make_index_sequence<delta>{})
        );
    }
}

}
