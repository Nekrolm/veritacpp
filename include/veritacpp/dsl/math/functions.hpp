#pragma once

#include <veritacpp/dsl/math/core_concepts.hpp>
#include <veritacpp/dsl/math/variable.hpp>
#include <veritacpp/dsl/math/constants.hpp>

#include <veritacpp/utils/tuple.hpp>

namespace veritacpp::dsl::math { 

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


}