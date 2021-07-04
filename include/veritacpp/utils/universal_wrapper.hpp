#pragma once

#include <type_traits>
#include <utility>
#include <concepts>
#include <functional>

namespace veritacpp::utils {

namespace detail {
struct owner_tag {};
struct reference_tag {};

template <typename Tag, typename T>
struct universal_wrapper;



template <typename T>
requires(!std::is_reference_v<T>) struct universal_wrapper<reference_tag, T> {
    explicit constexpr universal_wrapper(T& u) : value{&u} {}

    universal_wrapper(T&&) = delete;

    constexpr operator T&() { return *value; }

    constexpr operator const T&() const { return *value; }

    constexpr T& get() { return *value; }

    constexpr const T& get() const { return *value; }

    template <class... Args>
    requires std::is_invocable_v<T, Args...>
    decltype(auto) operator()(Args&&... args)  {
        return std::invoke(*value, std::forward<Args>(args)...);
    }
    template <class... Args>
    requires std::is_invocable_v<const T, Args...>
    decltype(auto) operator()(Args&&... args) const {
        return std::invoke(std::as_const(*value), std::forward<Args>(args)...);
    }

private:
    T* value;
};


template <typename T>
requires(!std::is_reference_v<T>) struct universal_wrapper<owner_tag, T> {
    constexpr universal_wrapper(T&& u) : value{std::move(u)} {}

    explicit constexpr universal_wrapper(
        const T& u) requires std::copy_constructible<T> : value{value} {}

    constexpr operator T&() & { return value; }
    constexpr operator const T&() const& { return value; }
    constexpr operator T&&() && { return std::move(value); }

    constexpr T& get() & { return value; }

    constexpr const T& get() const& { return value; }

    constexpr T&& get() && { return std::move(value); }

    template <class... Args>
    requires std::is_invocable_v<T, Args...>
    decltype(auto) operator()(Args&&... args) {
        return std::invoke(value, std::forward<Args>(args)...);
    }
    template <class... Args>
    requires std::is_invocable_v<const T, Args...>
    decltype(auto) operator()(Args&&... args) const {
        return std::invoke(value, std::forward<Args>(args)...);
    }

    constexpr auto ref() const & {
        return universal_wrapper<reference_tag, const T>{get()};
    }

    constexpr auto ref() & {
        return universal_wrapper<reference_tag, const T>{get()};
    }
    void ref() && = delete;

    constexpr auto mutable_ref() & requires (!std::is_const_v<T>)
    {
        return universal_wrapper<reference_tag, T>{get()};
    }
    void mutable_ref() && = delete;


private:
    T value;
};


template <typename T>
struct infer_universal_wrapper {
    using tag_type = std::conditional_t<std::is_lvalue_reference_v<T>, 
                                        reference_tag, owner_tag>;
    using value_type = std::remove_reference_t<T>;
    using type = universal_wrapper<tag_type, value_type>;
};

template <typename T>
using infer_universal_wrapper_t = typename infer_universal_wrapper<T>::type;

}  // namespace detail

template <class T>
using Owner = detail::universal_wrapper<detail::owner_tag, T>;

template <class T>
using Reference = detail::universal_wrapper<detail::reference_tag, T>;

namespace detail {

template <class T>
struct is_universal_wrapper : std::false_type {};

template <class T>
struct is_universal_wrapper<Owner<T>> : std::true_type {};

template <class T>
struct is_universal_wrapper<Reference<T>> : std::true_type {};

}

template <class T>
concept UniversalWrapper = detail::is_universal_wrapper<T>::value;

template <class T, class U>
concept UniversalWrapperFor = UniversalWrapper<T> &&
                              (std::same_as<T, Owner<U>> || std::same_as<T, Reference<U>>);


template <typename T>
constexpr UniversalWrapperFor<std::remove_reference_t<T>> auto universal_forward(T&& t) {
    return detail::infer_universal_wrapper_t<T>(std::forward<T>(t));
}

template <typename T>
constexpr Reference<std::remove_reference_t<T>> ref(T&& t)
requires std::is_lvalue_reference_v<decltype(t)> {
    return universal_forward(t);
} 

}  // namespace veritacpp::utils