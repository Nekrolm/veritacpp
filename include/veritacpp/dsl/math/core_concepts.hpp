#pragma once

#include <concepts>
#include <type_traits>

namespace veritacpp::dsl::math {

struct BasicFunction {};

template <class T>
concept Functional = std::is_base_of_v<BasicFunction, T>;

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;
    
} // veritacpp::dsl::math