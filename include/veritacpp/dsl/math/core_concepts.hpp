#pragma once

#include <concepts>
#include <type_traits>
#include <array>

namespace veritacpp::dsl::math {

struct BasicFunction {};

template <class T>
concept Functional = std::is_base_of_v<BasicFunction, T>;

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;

namespace detail {


template <uint64_t N, class F>
constexpr bool is_invocable_with_N_arithmetics = []<uint64_t... idx>(
        std::integer_sequence<uint64_t, idx...>){
        return std::is_invocable_v<F, decltype(static_cast<float>(idx))...>;
    }(std::make_index_sequence<N>{});
}

template <uint64_t N, class T>
concept NVariablesFunctional = Functional<T> && detail::is_invocable_with_N_arithmetics<N, T>;
    

} // veritacpp::dsl::math