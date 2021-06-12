#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <tuple>
#include <utility>
#include <cmath>

#include <veritacpp/dsl/math/core_concepts.hpp>
#include <veritacpp/dsl/math/variable.hpp>
#include <veritacpp/dsl/math/functions.hpp>
#include <veritacpp/dsl/math/constants.hpp>

#include <veritacpp/utils/tuple.hpp>

namespace veritacpp::dsl::math {



namespace detail {
template <class T>
struct IsVariable : std::false_type {};

template <uint64_t N>
struct IsVariable<Variable<N>> : std::true_type {};
}

template <class T>
concept DifferentialVariable = detail::IsVariable<T>::value;


template <Arithmetic auto C, DifferentialVariable X>
constexpr Functional auto diff(Constant<C>, X x) {
    return kZero;
}

template <Arithmetic T, DifferentialVariable X>
constexpr Functional auto diff(RTConstant<T>, X x) {
    return kZero;
}

template <uint64_t N, uint64_t M>
constexpr Functional auto diff(Variable<N>, Variable<M>) {
    if constexpr( N == M) {
        return kOne;
    } else {
        return kZero;
    }
}


template<Functional F, DifferentialVariable X>
constexpr Functional auto diff(Negate<F> nf, X x) {
    return -diff(nf.f, x);
}




template<Functional A, Functional B, DifferentialVariable X>
constexpr Functional auto diff(Add<A, B> s, X x) {
    return diff(s.f1, x) + diff(s.f2, x);
}


template<Functional A, Functional B, DifferentialVariable X>
constexpr Functional auto diff(Sub<A, B> s, X x) {
    return diff(s.f1, x) - diff(s.f2, x);
}


template <Functional F1, Functional F2, DifferentialVariable X>
constexpr Functional auto diff(Mul<F1, F2> m, X x) { 
    return diff(m.f1, x) * m.f2 + m.f1 * diff(m.f2, x);
};


template <Functional F1, Functional F2, DifferentialVariable X>
constexpr Functional auto diff(Div<F1, F2> d, X x) { 
    return (diff(d.f1, x) * d.f2 - d.f1 * diff(d.f2, x)) / (d.f2 * d.f2);
};


namespace detail {

template <Functional F, DifferentialVariable X, Functional... Gs>
static constexpr Functional auto apply_chain_rule(F f, X x, Gs... gs) {
    const auto g_binging = (gs, ...);
    constexpr auto g_cnt = sizeof...(gs);
    const auto g_tuple = std::make_tuple(gs...);

    const auto dgi_dx = [&g_tuple, x]<uint64_t idx>(Variable<idx> gi) {
        if constexpr (g_cnt > idx) {
            return diff(std::get<idx>(g_tuple), x);
        } else {
            return diff(gi, x);
        }
    };

    const auto df_dgi_dgi_dx = [&]<uint64_t idx>(Variable<idx> y) {
        return (diff(f, y) | g_binging) * dgi_dx(y); 
    };

    const auto chain = [&]<uint64_t... idx>(std::integer_sequence<uint64_t, idx...>) {
        return (df_dgi_dgi_dx(Variable<idx>{}) + ...);
    };
    
    
    const auto left_part = chain(std::make_index_sequence<g_cnt>{});
    if constexpr (g_cnt <= X::Id) {
        return left_part + df_dgi_dgi_dx(x);
    } else {
        return left_part;
    }
}

}

template <Functional F, Functional... Gs, DifferentialVariable X>
constexpr Functional auto diff(App<F, Gs...> ap, X x) {
    return std::apply([&](Gs... gs){
         return detail::apply_chain_rule(ap.f, x, gs...);
    },  ap.gs);
};




template <Arithmetic auto C, uint64_t xid>
constexpr Functional auto diff(Pow<C>, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    if constexpr (C == 0) {
        return kOne;
    }
    return Constant<C>{} * Pow<C - 1>{};
}


template <Arithmetic T, uint64_t xid>
constexpr Functional auto diff(RTPow<T> pw, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return  pw.deg * RTPow(RTConstant<T>{pw.deg() - 1});
}


template <uint64_t xid>
constexpr Functional auto diff(Sin, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return Cos{};
}
template <uint64_t xid>
constexpr Functional auto diff(Cos, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return -Sin{};
}

template <uint64_t xid>
constexpr Functional auto diff(Exp, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return Exp{};
}
template <uint64_t xid>
constexpr Functional auto diff(Log, Variable<xid> x) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return kOne / x;
}



} // veritacpp::dsl