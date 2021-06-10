#pragma once

#include <concepts>
#include <type_traits>

namespace veritacpp::dsl::math {

struct DifferentialBase {};

template <class T>
concept Differentiable = std::is_base_of_v<DifferentialBase, T>;

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;
    
} // veritacpp::dsl::math