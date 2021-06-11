#pragma once

#include <veritacpp/dsl/math/core_concepts.hpp>

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

}