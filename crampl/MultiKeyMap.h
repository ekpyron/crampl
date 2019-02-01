/**
 *
 *
 * @file MultiKeyMap.h
 * @brief 
 * @author clonker
 * @date 2/1/19
 */
#pragma once

#include <map>

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


template<template<typename...> typename MapKind, typename... KeyTypes___And___ValueType>
using MultiKeyMap = typename detail::MultiKeyMap<MapKind, KeyTypes___And___ValueType...>::type;

}
