#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <tuple>
#include <utility>
#include <cmath>

#include <veritacpp/utils/tuple.hpp>

namespace veritacpp::dsl {

struct DifferentialBase {};

template <class T>
concept Differentiable = std::is_base_of_v<DifferentialBase, T>;

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;

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


template <uint64_t N>
struct Variable : DifferentialBase {
    static constexpr auto Id = N;

    constexpr Arithmetic auto operator()(Arithmetic auto... args) const 
    requires (sizeof...(args) > N)
    {
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


template <Arithmetic auto C, DifferentialVariable X>
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

template <Differentiable F>
struct Negate : DifferentialBase {
    F f;
    explicit constexpr Negate(F f) : f(f) {}

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const {
        return -(f(x...));
    } 
};

template<Differentiable F>
constexpr Differentiable auto operator - (F f) {
    return Negate<F> { f };
}

template<Differentiable F>
constexpr Differentiable auto operator + (F f) {
    return f;
}

template<Differentiable F, DifferentialVariable X>
constexpr Differentiable auto diff(Negate<F> nf, X x) {
    return -diff(nf.f, x);
}


template<Differentiable F1, Differentiable F2>
struct Add : DifferentialBase {
    F1 f1;
    F2 f2;
    explicit constexpr Add(F1 f1, F2 f2) : f1{f1}, f2{f2} {};

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const {
        return f1(x...) + f2(x...);
    }
};

constexpr Differentiable auto operator + (Differentiable auto a, 
                                          Differentiable auto b) {
    return Add { a, b };
}

template<Differentiable A, Differentiable B, DifferentialVariable X>
constexpr Differentiable auto diff(Add<A, B> s, X x) {
    return diff(s.f1, x) + diff(s.f2, x);
}


template<Differentiable F1, Differentiable F2>
struct Sub : DifferentialBase {
    F1 f1;
    F2 f2;
    explicit constexpr Sub(F1 f1, F2 f2) : f1{f1}, f2{f2} {};

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const {
        return f1(x...) - f2(x...);
    }
};

constexpr Differentiable auto operator - (Differentiable auto a, 
                                          Differentiable auto b) {
    return Sub { a, b };
}

template<Differentiable A, Differentiable B, DifferentialVariable X>
constexpr Differentiable auto diff(Sub<A, B> s, X x) {
    return diff(s.f1, x) - diff(s.f2, x);
}


template<Differentiable F1, Differentiable F2>
struct Mul : DifferentialBase {
   F1 f1;
   F2 f2;

   constexpr Mul(F1 f1, F2 f2) : f1{f1}, f2{f2} {}

   constexpr Arithmetic auto operator()(Arithmetic auto... x) const {
       return f1(x...) * f2(x...);
   }
};


constexpr Differentiable auto operator * (Differentiable auto f1, 
                                          Differentiable auto f2) {
    return Mul { f1, f2 };
}


template <Differentiable F1, Differentiable F2, DifferentialVariable X>
constexpr Differentiable auto diff(Mul<F1, F2> m, X x) { 
    return diff(m.f1, x) * m.f2 + m.f1 * diff(m.f2, x);
};


template <Differentiable F1, Differentiable F2>
struct Div : DifferentialBase {
    F1 f1;
    F2 f2;
    constexpr Div(F1 f1, F2 f2) : f1{f1}, f2{f2} {}

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const {
       return f1(x...) / f2(x...);
    }
};

constexpr Differentiable auto operator / (Differentiable auto f1, 
                                          Differentiable auto f2) {
    return Div { f1, f2 };
}

template <Differentiable F1, Differentiable F2, DifferentialVariable X>
constexpr Differentiable auto diff(Div<F1, F2> d, X x) { 
    return (diff(d.f1, x) * d.f2 - d.f1 * diff(d.f2, x)) / (d.f2 * d.f2);
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


constexpr detail::BindingGroup auto operator , (Differentiable auto b1,
                                                Differentiable auto b2) {
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

template <Arithmetic auto C>
struct Pow : DifferentialBase {
    constexpr Arithmetic auto operator()(Arithmetic auto x, 
                                         Arithmetic auto...) const {
       return std::pow(x, C);
    }
};

template <Arithmetic auto C, uint64_t xid>
constexpr Differentiable auto diff(Pow<C>, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    if constexpr (C == 0) {
        return kOne;
    }
    return Constant<C>{} * Pow<C - 1>{};
}



template <Arithmetic auto C>
constexpr Differentiable auto operator ^ (Differentiable auto f, Constant<C>) {
    return Pow<C>{} | f;
}


template <Arithmetic T>
struct RTPow : DifferentialBase {

    const RTConstant<T> deg;
    explicit constexpr RTPow(RTConstant<T> deg) : deg { deg } {}

    constexpr Arithmetic auto operator()(Arithmetic auto x, 
                                         Arithmetic auto...) const {
       return std::pow(x, deg());
    }
};

template <Arithmetic T, uint64_t xid>
constexpr Differentiable auto diff(RTPow<T> pw, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return  pw.deg * RTPow(RTConstant<T>{pw.deg() - 1});
}



template <Arithmetic T>
constexpr Differentiable auto operator ^ (Differentiable auto f, 
                                          RTConstant<T> c) {
    return RTPow<T>{c} | f;
} 

template <Arithmetic T, Differentiable F>
constexpr Differentiable auto operator ^ (F f, T c) {
    return f ^ RTConstant<T> { c };
} 


template<Differentiable F, Arithmetic T>
constexpr auto operator + (F f,  T c) {
    return f + RTConstant { c }; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator * (F f,  T c) {
    return f * RTConstant { c }; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator - (F f,  T c) {
    return f - RTConstant { c }; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator / (F f,  T c) {
    return f / RTConstant { c }; 
}

template<Differentiable F, Arithmetic T>
constexpr auto operator + (T c, F f) {
    return  RTConstant { c } + f; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator * (T c, F f) {
    return  RTConstant { c } * f; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator - (T c, F f) {
    return  RTConstant { c } - f; 
}
template<Differentiable F, Arithmetic T>
constexpr auto operator / (T c, F f) {
    return  RTConstant { c } / f; 
}


struct Sin : DifferentialBase {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::sin(x);
    }
};

struct Cos : DifferentialBase {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::cos(x);
    }
};


template <uint64_t xid>
constexpr Differentiable auto diff(Sin, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return Cos{};
}
template <uint64_t xid>
constexpr Differentiable auto diff(Cos, Variable<xid>) {
    if constexpr(xid != 0) {
        return kZero;
    }
    return -Sin{};
}

constexpr Differentiable auto sin(Differentiable auto f) {
    return Sin{} | f;
}

constexpr Differentiable auto cos(Differentiable auto f) {
    return Cos{} | f;
}

} // veritacpp::dsl