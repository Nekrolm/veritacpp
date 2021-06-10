#pragma once

#include <veritacpp/dsl/math/core_concepts.hpp>


namespace veritacpp::dsl::math { 

template <Arithmetic auto C>
struct Constant : DifferentialBase {
    constexpr Arithmetic auto operator() (Arithmetic auto...) const {
        return C;
    }
};

template<Arithmetic auto C1, Arithmetic auto C2>
constexpr auto operator + (Constant<C1>, Constant<C2> c2) {
    return Constant<C1 + C2>{};
}
template<Arithmetic auto C1, Arithmetic auto C2>
constexpr auto operator * (Constant<C1>, Constant<C2> c2) {
    return Constant<C1 * C2>{};
}
template<Arithmetic auto C1, Arithmetic auto C2>
constexpr auto operator / (Constant<C1>, Constant<C2> c2) {
    return Constant<C1 / C2>{};
}
template<Arithmetic auto C1, Arithmetic auto C2>
constexpr auto operator - (Constant<C1>, Constant<C2> c2) {
    return Constant<C1 - C2>{};
}
template<Arithmetic auto C1, Arithmetic auto C2>
constexpr auto operator ^ (Constant<C1>, Constant<C2> c2) {
    return Constant<std::pow(C1,  C2)>{};
}


constexpr auto kZero = Constant<0>{};
constexpr auto kOne = Constant<1>{};

template <Arithmetic T>
struct RTConstant : DifferentialBase {
    const T value;
    explicit constexpr RTConstant(T val) : value(val) {} 

    constexpr Arithmetic auto operator() (Arithmetic auto...) const {
        return value;
    }
};

template<Arithmetic T1, Arithmetic T2>
constexpr auto operator + (RTConstant<T1> c1, RTConstant<T2> c2) {
    return RTConstant { c1.value + c2.value };
}
template<Arithmetic T1, Arithmetic T2>
constexpr auto operator * (RTConstant<T1> c1, RTConstant<T2> c2) {
    return RTConstant { c1.value * c2.value };
}
template<Arithmetic T1, Arithmetic T2>
constexpr auto operator - (RTConstant<T1> c1, RTConstant<T2> c2) {
    return RTConstant { c1.value - c2.value };
}
template<Arithmetic T1, Arithmetic T2>
constexpr auto operator / (RTConstant<T1> c1, RTConstant<T2> c2) {
    return RTConstant { c1.value / c2.value };
}
template<Arithmetic T1, Arithmetic T2>
constexpr auto operator ^ (RTConstant<T1> c1, RTConstant<T2> c2) {
    return RTConstant { std::pow(c1.value,  c2.value) };
}


constexpr RTConstant<double> operator "" _c (unsigned long long x) {
    return RTConstant { static_cast<double>( x ) };
}


}