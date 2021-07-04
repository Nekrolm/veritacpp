#include <veritacpp/utils/universal_wrapper.hpp>

#include <string>
#include <iostream>


int main(){

    std::string s1 = "hello world";

    const std::string s2 = "hello world";

    auto owner = veritacpp::utils::universal_forward(std::move(s1));
    auto ref = veritacpp::utils::universal_forward(s2);


    std::cout << ref.get();
    std::cout << owner.mutable_ref().get();
}