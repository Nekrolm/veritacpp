#pragma once

#include <veritacpp/dsl/math/core_concepts.hpp>
#include <veritacpp/dsl/math/constants.hpp>

namespace veritacpp::dsl::math {

template <uint64_t N>
struct Variable : BasicFunction {
    static constexpr auto Id = N;

    constexpr Arithmetic auto operator()(Arithmetic auto... args) const 
    requires (sizeof...(args) > N)
    {
        return std::get<N>(std::make_tuple(args...));
    }
};


template <uint64_t N, uint64_t M>
requires (N == M)
Functional auto operator - (Variable<N>, Variable<M>) {
    return kZero;
}

template <uint64_t N, uint64_t M>
requires (N == M)
Functional auto operator / (Variable<N>, Variable<M>) {
    return kOne;
}

}