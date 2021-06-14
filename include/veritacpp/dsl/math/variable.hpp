#pragma once

#include <algorithm>

#include <veritacpp/dsl/math/core_concepts.hpp>
#include <veritacpp/dsl/math/constants.hpp>

namespace veritacpp::dsl::math {

template <uint64_t N>
struct Variable;

template<uint64_t N, Functional F>
struct VariableBindingHolder {
    F f;
    static constexpr auto VariableID = N;
    
    constexpr VariableBindingHolder(Variable<N>, F f) : f{f} {}

    constexpr Functional auto operator()(Variable<N>) const {
        return f;
    }
};

template <uint64_t N, Functional F>
VariableBindingHolder(Variable<N>, F) -> VariableBindingHolder<N, F>;


template <class T>
struct IsVariableBinding : std::false_type {};

template <uint64_t N, Functional F>
struct IsVariableBinding<VariableBindingHolder<N, F>> : std::true_type {};

template <class T>
concept VariableBinding = IsVariableBinding<T>::value;

template <VariableBinding... Var> 
struct VariableBindingGroup : Var... {
    using Var::operator()...;

    static constexpr auto MaxVariableID = std::max({ Var::VariableID... });

    template<uint64_t N> requires ((N != Var::VariableID) && ...)
    constexpr Functional auto operator ()  (Variable<N> x) const {
        return x;
    }
};

template <VariableBinding... Var>
VariableBindingGroup(Var...) -> VariableBindingGroup<Var...>;


template <uint64_t N>
struct Variable : BasicFunction {
    static constexpr auto Id = N;

    constexpr Arithmetic auto operator()(Arithmetic auto... args) const 
    requires (sizeof...(args) > N)
    {
        return std::get<N>(std::make_tuple(args...));
    }

    template <Functional F>
    constexpr auto operator = (F f) const {
        return VariableBindingGroup { VariableBindingHolder<N, F>(*this, f) };
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

// Veriable<N> requires at least N+1 arguments
static_assert(!(NVariablesFunctional<2, Variable<2>>));





template <VariableBinding... Var>
constexpr bool all_unique_variables = []<uint64_t... idx>(
    std::integer_sequence<uint64_t, idx...>){
        constexpr std::array arr { idx...}; 
        return ((std::ranges::count(arr, idx) == 1) && ...);
    }(std::integer_sequence<uint64_t, Var::VariableID...>{});

template <VariableBinding... Var> 
consteval bool group_contains_binding(VariableBindingGroup<Var...>, VariableBinding auto v) {
    return ((Var::VariableID == v.VariableID) || ...);
}


template<VariableBinding... V1, VariableBinding... V2>
constexpr auto operator , (VariableBindingGroup<V1...> v1, VariableBindingGroup<V2...> v2) {
    static_assert(all_unique_variables<V1..., V2...>,
                 "rebinding same variable twice is not allowed");
    return VariableBindingGroup {
        static_cast<V1>(v1)...,
        static_cast<V2>(v2)...
    };
}



}