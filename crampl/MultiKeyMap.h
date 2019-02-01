/**
 *
 *
 * @file MultiKeyMap.h
 * @brief 
 * @author clonker
 * @date 2/1/19
 */
#pragma once

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

template<typename T>
struct MultiKeyMapValueType {
    using type = T;
};

template<template<typename...> typename MapKind, typename Key, typename Value, typename... Tail>
struct MultiKeyMapValueType<MapKind<Key, Value, Tail...>>: MultiKeyMapValueType<Value> {};

template<typename T>
using MultiKeyMapValueType_t = typename MultiKeyMapValueType<std::decay_t<T>>::type;

}


template<template<typename...> typename MapKind, typename... KeyTypes___And___ValueType>
using MultiKeyMap = typename detail::MultiKeyMap<MapKind, KeyTypes___And___ValueType...>::type;


template<typename A, typename B, typename... C>
std::optional<detail::MultiKeyMapValueType_t<A>> find(A&& map, B&& key, C&&... keys)
{
    if constexpr (sizeof...(keys) > 0) {
        auto it = map.find(key);
        if (it == map.end())
            return {};
        return find(it->second, keys...);
    } else {
        auto it = map.find(key);
        if (map.find(key) == map.end())
            return {};
        return { it->second };
    }
}

}
