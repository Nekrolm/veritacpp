#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <tuple>
#include <utility>

#include <veritacpp/utils/tuple.hpp>

namespace veritacpp::dsl {

struct DifferentialBase {};

template <class T>
concept Differentiable = std::is_base_of_v<DifferentialBase, T>;

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <auto C> requires Arithmetic<decltype(C)>
struct Constant : DifferentialBase {
    template <Arithmetic... Args>
    constexpr Arithmetic auto operator() (Args...) const {
        return C;
    }
};

constexpr auto kZero = Constant<0>{};
constexpr auto kOne = Constant<1>{};

template <Arithmetic T>
struct RTConstant : DifferentialBase {
    const T value;
    explicit constexpr RTConstant(T val) : value(val) {} 

    template <Arithmetic... Args>
    constexpr Arithmetic auto operator() (Args...) const {
        return value;
    }
};

constexpr RTConstant<double> operator "" _c (unsigned long long x) {
    return RTConstant { static_cast<double>( x ) };
}


template <uint64_t N>
struct Variable : DifferentialBase {
    static constexpr auto Id = N;

    template <Arithmetic... Args> requires (sizeof...(Args) > N)
    constexpr Arithmetic auto operator()(Args... args) const {
        return std::get<N>(std::make_tuple(args...));
    }
};


namespace detail {
template <class T>
struct IsVariable : std::false_type {};

template <uint64_t N>
struct IsVariable<Variable<N>> : std::true_type {};
}

template <class T>
concept DifferentialVariable = detail::IsVariable<T>::value;


template <auto C, DifferentialVariable X>
constexpr Differentiable auto diff(Constant<C>, X x) {
    return kZero;
}

template <Arithmetic T, DifferentialVariable X>
constexpr Differentiable auto diff(RTConstant<T>, X x) {
    return kZero;
}


template <uint64_t N, uint64_t M>
constexpr Differentiable auto diff(Variable<N>, Variable<M>) {
    if constexpr( N == M) {
        return kOne;
    } else {
        return kZero;
    }
}



template<Differentiable... F>
struct Sum : DifferentialBase {
    std::tuple<F...> fs;
    explicit constexpr Sum(F... f) : fs { f... } {};
    explicit constexpr Sum(std::tuple<F...> fs) : fs { std::move(fs)} {}

    template<Arithmetic... Args>
    constexpr Arithmetic auto operator()(Args... x) const {
        auto args = std::make_tuple(x...);
        return std::apply([&args](auto&&... f){
            return (std::apply(f, args) + ...);
        }, fs);
    }
};

template <Differentiable A, Differentiable B>
constexpr Differentiable auto operator + (A a, B b) {
    return Sum { a, b };
}

template<Differentiable... F, DifferentialVariable X>
constexpr Differentiable auto diff(Sum<F...> s, X x) {
    return std::apply([x](auto... f){
              return (diff(f, x) + ...);
           }, s.fs);
}


template<Differentiable F1, Differentiable F2>
struct Mul : DifferentialBase {
   F1 f1;
   F2 f2;

   constexpr Mul(F1 f1, F2 f2) : f1{f1}, f2{f2} {}

   template <Arithmetic... Args>
   constexpr Arithmetic auto operator()(Args... x) const {
       return f1(x...) * f2(x...);
   }
};

template<Differentiable F1, Differentiable F2>
Mul(F1, F2) -> Mul<F1, F2>;

template<Differentiable F1, Differentiable F2>
constexpr Differentiable auto operator * (F1 f1, F2 f2) {
    return Mul { f1, f2 };
}


template <Differentiable F1, Differentiable F2, DifferentialVariable X>
constexpr Differentiable auto diff(Mul<F1, F2> m, X x) { 
        return  diff(m.f1, x) * m.f2 + m.f1 * diff(m.f2, x);
};

template <Differentiable F, Differentiable... Gs>
struct App : DifferentialBase {
    F f;
    std::tuple<Gs...> gs;
    constexpr explicit App(F f, Gs... gs) : f(f), gs{gs...} {}
    constexpr explicit App(F f, std::tuple<Gs...> gs) : f(f), gs{gs} {}
    

    template<Arithmetic... Args>
    constexpr Arithmetic auto operator()(Args... x) const {
        auto args = std::make_tuple(x...);
        auto leftmost_args =  std::apply([&args](auto&&... g){
            return std::make_tuple(apply(g, args)...);
        }, gs);
        auto [ignore, rightmost_args] = veritacpp::utils::split<sizeof...(x)>(args);
        return std::apply(f, std::tuple_cat(leftmost_args, rightmost_args));
    }  
};


namespace detail {

template <Differentiable F, DifferentialVariable X, Differentiable... Gs>
static constexpr Differentiable auto apply_chain_rule(F f, X x, Gs... gs) {
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

template <Differentiable F, Differentiable... Gs, DifferentialVariable X>
constexpr Differentiable auto diff(App<F, Gs...> ap, X x) {
        return std::apply([&](Gs... gs){
            return detail::apply_chain_rule(ap.f, x, gs...);
        },  ap.gs);
};


namespace detail {

template <Differentiable... F>
struct BindingTuple : std::tuple<F...> {
    using std::tuple<F...>::tuple;

    constexpr std::tuple<F...> as_tuple() const {
        return *this;
    }

};

template <Differentiable... F>
BindingTuple(F...) -> BindingTuple<F...>;

template <Differentiable... F>
BindingTuple(std::tuple<F...>) -> BindingTuple<F...>;


template <class T>
struct IsBindingTuple : std::false_type {};

template <Differentiable... F>
struct IsBindingTuple<BindingTuple<F...>> : std::true_type {};

template <class T>
concept BindingGroup = IsBindingTuple<T>::value;

template <BindingGroup B1, BindingGroup B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(b1.as_tuple(), b2.as_tuple()) };
}

template <Differentiable B1, BindingGroup B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(std::make_tuple(b1), b2.as_tuple()) };
}

template <BindingGroup B1, Differentiable B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(b1.as_tuple(), std::make_tuple(b2)) };
}

} // detail


template <Differentiable B1, Differentiable B2>
constexpr detail::BindingGroup auto operator , (B1 b1, B2 b2) {
    return detail::BindingTuple(b1, b2);
}

template <Differentiable F, Differentiable G>
constexpr Differentiable auto operator | (F f, G g) {
    return App<F, G> { f, g };
}

template <Differentiable F, Differentiable... G>
constexpr Differentiable auto operator | (F f, detail::BindingTuple<G...> g) {
    return App<F, G...> { f, g.as_tuple() };
}


} // veritacpp::dsl