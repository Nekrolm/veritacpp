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

template <uint64_t N, class F, class = void>
struct IsInvocableWithNArithmetics : std::false_type {};

template <uint64_t N, class F>
struct IsInvocableWithNArithmetics<N, F, 
decltype(void(std::apply(std::declval<F>(), std::array<double, N>{})))
> : std::true_type {};

template <uint64_t N, class F>
constexpr bool is_invocable_with_N_arithmetics = requires (F f) {
    []<uint64_t... idx>(F f, std::integer_sequence<uint64_t, idx...>){
        f((idx, 0.f)...);
    }(f, std::make_index_sequence<N>{});
};
}

template <uint64_t N, class T>
concept NVariablesFunctional = Functional<T> && detail::is_invocable_with_N_arithmetics<N, T>;
    

} // veritacpp::dsl::math