/**
 *
 *
 * @file MultiKeyMap.h
 * @brief 
 * @author clonker
 * @date 2/1/19
 */
#pragma once

#include <algorithm>
#include <optional>
#include <utility>
#include <ranges>

namespace crampl {

template<typename Key, typename...>
struct ComplexKey {};

namespace detail {

template<template<typename...> typename MapKind, typename... KeyTypes___And___ValueType>
struct MultiKeyMap;

template<template<typename...> typename MapKind, typename Key, typename... KeyTypes___And___ValueType>
struct MultiKeyMap<MapKind, Key, KeyTypes___And___ValueType...> {
    using type = MapKind<Key, typename MultiKeyMap<MapKind, KeyTypes___And___ValueType...>::type>;
};

template<template<typename...> typename MapKind, typename Key, typename... KeyTypes___And___ValueType, typename... Tail>
struct MultiKeyMap<MapKind, ComplexKey<Key, Tail...>, KeyTypes___And___ValueType...> {
    using type = MapKind<Key, typename MultiKeyMap<MapKind, KeyTypes___And___ValueType...>::type, Tail...>;
};

template<template<typename...> typename MapKind, typename Key, typename Value>
struct MultiKeyMap<MapKind, Key, Value> {
    using type = MapKind<Key, Value>;
};

template<template<typename...> typename MapKind, typename Key, typename Value, typename... Tail>
struct MultiKeyMap<MapKind, ComplexKey<Key, Tail...>, Value> {
    using type = MapKind<Key, Value, Tail...>;
};
}


template<template<typename...> typename MapKind, typename... KeyTypes_AND_ValueType>
using MultiKeyMap = typename detail::MultiKeyMap<MapKind, KeyTypes_AND_ValueType...>::type;


template<typename A, typename B, typename... C>
auto find(A&& map, B&& key, C&&... keys)
{
    if constexpr (sizeof...(keys) > 0) {
        auto it = map.find(key);
        if (it == map.end())
            return std::optional<decltype(find(it->second, keys...))>{};
        return std::make_optional(find(it->second, keys...));
    } else {
        auto it = map.find(key);
        if (map.find(key) == map.end())
            return std::optional<decltype(it->second)>{};
        return std::make_optional(it->second);
    }
}

template<typename T>
struct compose {
    T lambda;
};

template<typename T>
compose(T) -> compose<T>;

template<typename Lambda1, typename Lambda2>
auto operator+(compose<Lambda1> lhs, compose<Lambda2> rhs)
{
    return compose{[&]<typename... Args>(Args&&... args) mutable -> decltype(auto)
    {
        return std::apply(std::move(lhs.lambda), std::move(rhs.lambda)(std::forward<Args>(args)...));
    }};
}


template<typename A,  typename... B>
auto& emplace(A& map, std::piecewise_construct_t, B&&... keys) {
    return [&]<auto... Ix>(std::index_sequence<Ix...>) -> decltype(auto) {
        return std::get<0>((... + compose{[&](auto& inner, auto&& key, auto&&... args) mutable -> decltype(auto) {
            static_cast<void>(Ix);
            if constexpr (sizeof...(args) == 1) {
                auto [it, _] = inner.emplace(std::piecewise_construct, static_cast<decltype(key)>(key), static_cast<decltype(args)>(args)...);
                return std::make_tuple(std::ref(it->second), std::nullopt);
            } else {
                auto [it, _] = inner.emplace(std::piecewise_construct, static_cast<decltype(key)>(key), std::forward_as_tuple());
                return std::make_tuple(std::ref(it->second), static_cast<decltype(args)>(args)...);
            }
        }}).lambda(map, std::forward<B>(keys)...));
    }(std::make_index_sequence<sizeof...(keys) - 1>{});
}

/*
template<typename A, typename B, typename... C>
detail::MultiKeyMapValueType_t<A> &emplace(A&& map, std::piecewise_construct_t, B&& key, C&&... keys)
{
    if constexpr (sizeof...(keys) > 1) {
        auto [it, _] = map.emplace(std::piecewise_construct, std::forward<B>(key), std::forward_as_tuple());
        return emplace(it->second, std::piecewise_construct, std::forward<C>(keys)...);
    } else {
        auto [it, _] = map.emplace(std::piecewise_construct, std::forward<B>(key), std::forward<C>(keys)...);
        return it->second;
    }
}
 */
/*
    template<typename A,  typename B1, typename... B>
    auto& emplace(A& map, std::piecewise_construct_t, B1&& _key1, B&&... keys) {
        return std::get<0>((... + compose{[&](auto& inner, auto&& key, auto&& value, auto&&... args) mutable -> decltype(auto) {
            static_cast<void>(keys);
            if constexpr (sizeof...(args) == 0) {
                auto [it, _] = inner.emplace(std::piecewise_construct, static_cast<decltype(key)>(key), static_cast<decltype(value)>(value));
                return std::make_tuple(std::ref(it->second));
            } else {
                auto [it, _] = inner.emplace(std::piecewise_construct, static_cast<decltype(key)>(key), std::forward_as_tuple());
                return std::make_tuple(std::ref(it->second), static_cast<decltype(value)>(value), static_cast<decltype(args)>(args)...);
            }
        }}).lambda(map, std::forward<B1>(_key1), std::forward<B>(keys)...));
    }
*/

/*
template<typename A,  typename B1, typename... B>
auto& emplace(A& map, std::piecewise_construct_t, B1&& _key1, B&&... keys) {
    return std::get<0>((... + compose{[&]<typename Map, typename Key, typename Value, typename... Args>(Map& inner, Key&& key, Value&& value, Args&&... args) mutable -> decltype(auto) {
        static_cast<void>(keys);
        if constexpr (sizeof...(args) == 0) {
            auto [it, _] = inner.emplace(std::piecewise_construct, std::forward<Key>(key), std::forward<Value>(value));
            return std::make_tuple(std::ref(it->second));
        } else {
            auto [it, _] = inner.emplace(std::piecewise_construct, std::forward<Key>(key), std::forward_as_tuple());
            return std::make_tuple(std::ref(it->second), std::forward<Value>(value), std::forward<Args>(args)...);
        }
    }}).lambda(map, std::forward<B1>(_key1), std::forward<B>(keys)...));
}
 */

}
