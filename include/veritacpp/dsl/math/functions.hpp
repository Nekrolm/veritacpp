#pragma once

#include <array>

#include <veritacpp/dsl/math/core_concepts.hpp>
#include <veritacpp/dsl/math/variable.hpp>
#include <veritacpp/dsl/math/constants.hpp>

#include <veritacpp/utils/tuple.hpp>

namespace veritacpp::dsl::math { 

template <Functional F>
struct Negate : BasicFunction {
    F f;
    explicit constexpr Negate(F f) : f(f) {}

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const 
    requires NVariablesFunctional<sizeof...(x), F>
    {
        return -(f(x...));
    } 
};

template<Functional F>
constexpr Functional auto operator - (F f) {
    return Negate<F> { f };
}

template<Functional F>
constexpr Functional auto operator + (F f) {
    return f;
}

template<Functional F1, Functional F2>
struct Add : BasicFunction {
    F1 f1;
    F2 f2;
    explicit constexpr Add(F1 f1, F2 f2) : f1{f1}, f2{f2} {};

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const 
    requires NVariablesFunctional<sizeof...(x), F1> 
          && NVariablesFunctional<sizeof...(x), F2> {
        return f1(x...) + f2(x...);
    }
};

constexpr Functional auto operator + (Functional auto a, 
                                      Functional auto b) {
    return Add { a, b };
}

template<Functional F1, Functional F2>
struct Sub : BasicFunction {
    F1 f1;
    F2 f2;
    explicit constexpr Sub(F1 f1, F2 f2) : f1{f1}, f2{f2} {};

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const
    requires NVariablesFunctional<sizeof...(x), F1> 
          && NVariablesFunctional<sizeof...(x), F2>
    {
        return f1(x...) - f2(x...);
    }
};

constexpr Functional auto operator - (Functional auto a, 
                                      Functional auto b) {
    return Sub { a, b };
}

template<Functional F1, Functional F2>
struct Mul : BasicFunction {
    F1 f1;
    F2 f2;

    constexpr Mul(F1 f1, F2 f2) : f1{f1}, f2{f2} {}

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const
    requires NVariablesFunctional<sizeof...(x), F1> 
          && NVariablesFunctional<sizeof...(x), F2>
    {
        return f1(x...) * f2(x...);
    }
};


constexpr Functional auto operator * (Functional auto f1, 
                                      Functional auto f2) {
    return Mul { f1, f2 };
}

template <Functional F1, Functional F2>
struct Div : BasicFunction {
    F1 f1;
    F2 f2;
    constexpr Div(F1 f1, F2 f2) : f1{f1}, f2{f2} {}

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const
    requires NVariablesFunctional<sizeof...(x), F1> 
          && NVariablesFunctional<sizeof...(x), F2> {
        return f1(x...) / static_cast<double>(f2(x...));
    }
};

constexpr Functional auto operator / (Functional auto f1, 
                                      Functional auto f2) {
    return Div { f1, f2 };
}

template <Functional F, Functional... Gs>
struct App : BasicFunction {
    F f;
    std::tuple<Gs...> gs;
    constexpr explicit App(F f, Gs... gs) : f(f), gs{gs...} {}
    constexpr explicit App(F f, std::tuple<Gs...> gs) : f(f), gs{gs} {}
    

    constexpr Arithmetic auto operator()(Arithmetic auto... x) const 
    requires NVariablesFunctional<std::max(sizeof...(Gs), sizeof...(x)), F> 
             && (NVariablesFunctional<sizeof...(x), Gs> && ...)
    {
        auto args = std::make_tuple(x...);
        auto leftmost_args =  std::apply([&args](auto... g){
            return std::make_tuple(apply(g, args)...);
        }, gs);
        auto [ignore, rightmost_args] = veritacpp::utils::split<sizeof...(Gs)>(args);
        return std::apply(f, std::tuple_cat(leftmost_args, rightmost_args));
    }  
};

namespace detail {

template <Functional... F>
struct BindingTuple : std::tuple<F...> {
    using std::tuple<F...>::tuple;

    constexpr std::tuple<F...> as_tuple() const {
        return *this;
    }

};

template <Functional... F>
BindingTuple(F...) -> BindingTuple<F...>;

template <Functional... F>
BindingTuple(std::tuple<F...>) -> BindingTuple<F...>;


template <class T>
struct IsBindingTuple : std::false_type {};

template <Functional... F>
struct IsBindingTuple<BindingTuple<F...>> : std::true_type {};

template <class T>
concept BindingGroup = IsBindingTuple<T>::value;

template <BindingGroup B1, BindingGroup B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(b1.as_tuple(), b2.as_tuple()) };
}

template <Functional B1, BindingGroup B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(std::make_tuple(b1), b2.as_tuple()) };
}

template <BindingGroup B1, Functional B2>
constexpr BindingGroup auto operator , (B1 b1, B2 b2) {
    return BindingTuple { std::tuple_cat(b1.as_tuple(), std::make_tuple(b2)) };
}

} // detail


constexpr detail::BindingGroup auto operator , (Functional auto b1,
                                                Functional auto b2) {
    return detail::BindingTuple(b1, b2);
}

template <Functional F, Functional G>
constexpr Functional auto operator | (F f, G g) {
    return App<F, G> { f, g };
}

template <Arithmetic T, Functional G>
constexpr Functional auto operator | (RTConstant<T> c, G) {
    return c;
}

template <Arithmetic T, Functional... G>
constexpr Functional auto operator | (RTConstant<T> c, detail::BindingTuple<G...>) {
    return c;
}

template <Arithmetic auto C, Functional... G>
constexpr Functional auto operator | (Constant<C> c, detail::BindingTuple<G...>) {
    return c;
}

template <Arithmetic auto C, Functional G>
constexpr Functional auto operator | (Constant<C> c, G) {
    return c;
}


template <Functional F, Functional... G>
constexpr Functional auto operator | (F f, detail::BindingTuple<G...> g) {
    return App<F, G...> { f, g.as_tuple() };
}

template <Arithmetic auto C>
struct Pow : BasicFunction {
    constexpr Arithmetic auto operator()(Arithmetic auto x, 
                                         Arithmetic auto...) const {
       return std::pow(x, C);
    }
};

template <Arithmetic auto C>
constexpr Functional auto operator ^ (Functional auto f, Constant<C>) {
    return Pow<C>{} | f;
}


template <Arithmetic T>
struct RTPow : BasicFunction {

    const RTConstant<T> deg;
    explicit constexpr RTPow(RTConstant<T> deg) : deg { deg } {}

    constexpr Arithmetic auto operator()(Arithmetic auto x, 
                                         Arithmetic auto...) const {
       return std::pow(x, deg());
    }
};


template <Arithmetic T>
constexpr Functional auto operator ^ (Functional auto f, 
                                      RTConstant<T> c) {
    return RTPow<T>{c} | f;
} 

template <Arithmetic T, Functional F>
constexpr Functional auto operator ^ (F f, T c) {
    return f ^ RTConstant<T> { c };
} 


template<Functional F, Arithmetic T>
constexpr auto operator + (F f,  T c) {
    return f + RTConstant { c }; 
}
template<Functional F, Arithmetic T>
constexpr auto operator * (F f,  T c) {
    return f * RTConstant { c }; 
}
template<Functional F, Arithmetic T>
constexpr auto operator - (F f,  T c) {
    return f - RTConstant { c }; 
}
template<Functional F, Arithmetic T>
constexpr auto operator / (F f,  T c) {
    return f / RTConstant { c }; 
}

template<Functional F, Arithmetic T>
constexpr auto operator + (T c, F f) {
    return  RTConstant { c } + f; 
}
template<Functional F, Arithmetic T>
constexpr auto operator * (T c, F f) {
    return  RTConstant { c } * f; 
}
template<Functional F, Arithmetic T>
constexpr auto operator - (T c, F f) {
    return  RTConstant { c } - f; 
}
template<Functional F, Arithmetic T>
constexpr auto operator / (T c, F f) {
    return  RTConstant { c } / f; 
}


struct Sin : BasicFunction {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::sin(x);
    }
};

struct Cos : BasicFunction {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::cos(x);
    }
};

struct Exp : BasicFunction {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::exp(x);
    }
};

struct Log : BasicFunction {
    template <Arithmetic X, Arithmetic... Args>
    constexpr Arithmetic auto operator()(X x, Args...) const {
       return std::log(x);
    }
};

constexpr Functional auto sin(Functional auto f) {
    return Sin{} | f;
}

constexpr Functional auto cos(Functional auto f) {
    return Cos{} | f;
}

constexpr Functional auto exp(Functional auto f) {
    return Exp{} | f;
}

constexpr Functional auto log(Functional auto f) {
    return Log{} | f;
}

// simplification for constants
template <Arithmetic X>
constexpr Functional auto sin(X x) {
    return RTConstant { Sin{}(x) }; 
}
template <Arithmetic X>
constexpr Functional auto sin(RTConstant<X> x) {
    return RTConstant { Sin{}(x()) }; 
}
template <Arithmetic auto X>
constexpr Functional auto sin(Constant<X>) {
    return Constant<Sin{}(X)>{}; 
}
template <Arithmetic X>
constexpr Functional auto cos(X x) {
    return RTConstant { Cos{}(x) }; 
}
template <Arithmetic X>
constexpr Functional auto cos(RTConstant<X> x) {
    return RTConstant { Cos{}(x()) }; 
}
template <Arithmetic auto X>
constexpr Functional auto cos(Constant<X>) {
    return Constant<Cos{}(X)>{}; 
}
template <Arithmetic X>
constexpr Functional auto exp(X x) {
    return RTConstant { Exp{}(x) }; 
}
template <Arithmetic X>
constexpr Functional auto exp(RTConstant<X> x) {
    return RTConstant { Exp{}(x()) }; 
}
template <Arithmetic auto X>
constexpr Functional auto exp(Constant<X>) {
    return Constant<Exp{}(X)>{};
}
template <Arithmetic X>
constexpr Functional auto log(X x) {
    return RTConstant { Log{}(x) }; 
}
template <Arithmetic X>
constexpr Functional auto log(RTConstant<X> x) {
    return RTConstant { Log{}(x()) }; 
}
template <Arithmetic auto X>
requires (X > 0)
constexpr Functional auto log(Constant<X>) {
    return Constant<Log{}(X)>{};
}


constexpr Functional auto operator ^ (Functional auto f,
                                      Functional auto g) {
    return exp(log(f) * g);
}

}