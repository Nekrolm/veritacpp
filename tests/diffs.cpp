#include <veritacpp/dsl/math/differential.hpp>
#include <veritacpp/dsl/math/constants.hpp>

#include <iostream>

int main() {

    using namespace veritacpp::dsl::math;

    constexpr auto x = Variable<0>{};
    constexpr auto y = Variable<1>{};
    constexpr auto z = Variable<2>{};
    
    constexpr auto f = x * y;
    constexpr auto g = x + x;

    constexpr auto binding = (x=f, y=g);

    static_assert(binding(x)(3,5) == 3 * 5); // f;
    static_assert(binding(y)(5) == 5 + 5); // g
    static_assert(binding(z)(0,0,0) == 0); // z
    static_assert(binding(z)(0,0,4) == 4); // z

   

    {
        constexpr auto f = sin(x + y);
        constexpr auto g = f | (y=2*x, x=3*x);
        static_assert(g(4) == sin(5*x)(4));
    }

    // constexpr auto wrong_binding = (x=f, x=g);

    constexpr auto x_y = x + y;
    static_assert((x_y)(5, 4) == 9);

    static_assert(diff(x_y, x)(1, 0) == 1);
    static_assert(diff(x_y, z)(1, 0) == 0);

    constexpr auto xy = x * y;

    static_assert(diff(xy, x)(0, 5) == 5);
    static_assert(diff(xy, y)(3, 5) == 3);

    constexpr auto c5x2 = 5_c * x * x;;

    static_assert(diff(c5x2, x)(10) == 100);


    constexpr auto c4x2 = 4_c * x * x;
    constexpr auto x2 = x * x;

    constexpr auto  x_y2 = x2 | (x + y);

    static_assert(x_y2(2,3) == 25); // (2 + 3)^2

    constexpr auto c9x2 = x_y2 | (x, 2_c * x);

    static_assert(c9x2(5) == 9 * 5 * 5);

    constexpr auto dc9x2dx = diff(c9x2, x);

    static_assert(dc9x2dx(1) == 18);
    constexpr auto dc9x2dy = diff(c9x2, y);
    static_assert(dc9x2dy(1, 1) == 0);


    constexpr auto neg_x2 = -x * x;

    static_assert(neg_x2(5) == -25);
    static_assert(diff(neg_x2, x)(5) == -10);

    constexpr auto x_sub_y = x - y;
    static_assert(x_sub_y(5,6) == -1);

    constexpr auto x_div_y = x / y;

    static_assert(diff(x_div_y, y)(1, 0.5) == -4);

    {
        constexpr auto x3 = x^3;
        static_assert(x3(5) == 125);
        static_assert(diff(x3, x)(5) == 3 * 25);
    }

    {
        constexpr auto poly = (x^2) + (x^3) - 5*x + 3;
        static_assert(poly(2) == 2*2 + 2*2*2 - 5*2 + 3);
        constexpr auto dpoly = diff(poly, x);
        static_assert(dpoly(2) == 2*2 + 3*2*2 - 5);
        
    }

    {
        constexpr auto trig_eq = (sin(x) ^ 2) + (cos(x) ^ 2);
        static_assert(trig_eq(0) == 1);
        static_assert(trig_eq(2) == 1);
        static_assert(std::abs(trig_eq(3) - 1) < 1e-6); // precision error :(

        static_assert(diff(sin(x), x)(0) == cos(x)(0)); 
        static_assert(diff(sin(x), x)(2) == cos(x)(2));
        static_assert(diff(cos(x), x)(2) == (-sin(x))(2));       
    }


    {
        constexpr auto x_pow_x = x ^ x;
        static_assert(x_pow_x(2) == 4);
        constexpr auto dx_pow_x = diff(x_pow_x, x);
        static_assert(dx_pow_x(2) == ((x^x) * (log(x) + 1))(2));
    }
    

    {
        constexpr auto x_add_y = x + y;
        constexpr auto f = x_add_y | (x=sin(y), y=x^2);
        static_assert(f(3,4) == std::sin(4) + 3*3);
    }

}