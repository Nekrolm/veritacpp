#include <veritacpp/dsl/differential.hpp>
#include <veritacpp/utils/tuple.hpp>

#include <iostream>

int main() {
    using namespace veritacpp::dsl;

    constexpr auto x = Variable<0>{};
    constexpr auto y = Variable<1>{};
    constexpr auto z = Variable<2>{};
    
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
}