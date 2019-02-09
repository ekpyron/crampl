/**
 *
 *
 * @file ContainerContainer.h
 * @brief 
 * @author clonker
 * @date 2/8/19
 */
#pragma once

#include <tuple>

namespace crampl {


namespace detail {
template<class F, class...Ts, std::size_t...Is>
constexpr void for_each_in_tuple_impl(std::tuple<Ts...> &tuple, F func, std::index_sequence<Is...>) {
    using expander = int[];
    (void) expander {0, ((void) func(std::get<Is>(tuple)), 0)...};
}

template<class F, class...Ts>
constexpr void for_each_in_tuple(std::tuple<Ts...> &tuple, F func) {
    for_each_in_tuple_impl(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}
/*
template<typename Container, std::size_t = 0>
struct has_compile_time_size: std::false_type {};

template<typename Container>
constexpr std::size_t getSizeOf() {
    Container container{};
    return container.size() - container.size();
}

template<typename Container>
struct has_compile_time_size<Container, getSizeOf<Container>()> {};

template<typename Container>
struct compile_time_size_tag {
    static constexpr std::size_t N = []() { Container c; return c.size(); } ();
};

template<typename Container>
constexpr std::size_t count_compile_time_sizes() {
    Container container;
    constexpr std::size_t N = container.size();
    return 1;
}

template<typename Container>
constexpr std::size_t count_compile_time_sizes() {
    return 0;
}

*/


template <class T, size_t N = T{}.size()>
constexpr inline std::optional<std::size_t> constexpr_size2(const T &t) {
    return {t.size()};
}
constexpr inline std::optional<std::size_t> constexpr_size2(...) {
    return {};
}

template <class T, size_t N = T{}.size()>
constexpr inline std::true_type constexpr_size(const T &) {
    return {};
}
constexpr inline std::false_type constexpr_size(...) {
    return {};
}

template<typename T>
constexpr bool has_constexpr_size = decltype(constexpr_size(T{}))::value;

template<typename Container, typename... Tail>
struct has_compile_time_sized_container {
private:
    static constexpr bool getValue() {
        if constexpr (sizeof...(Tail) > 0) {
            return has_constexpr_size<Container> || has_compile_time_sized_container<Tail...>::value;
        } else {
            return has_constexpr_size<Container>;
        }
    }

public:
    static constexpr bool value = getValue();

    static constexpr std::size_t size(std::tuple<Container&, Tail&...> containers) {
        return size(containers, std::make_index_sequence<sizeof...(Tail)>());
    }
    template<std::size_t... N>
    static constexpr std::size_t size(std::tuple<Container&, Tail&...> containers, std::index_sequence<N...>) {
        if constexpr (constexpr_size2<Container>(std::get<0>(containers))) {
            return *constexpr_size2<Container>(std::get<0>(containers));
        } else {
            return has_compile_time_sized_container<Tail...>::size(std::get<N>(containers)...);
            /*if constexpr (sizeof...(Tail) > 0) {
            } else {
                return Container{}.size();
            }*/
        }
    }
};


}

template<typename... Containers>
class ContainerContainer {
    static constexpr void sizeCheck(std::tuple<Containers&...> containers) {
        static_assert(detail::has_constexpr_size<std::array<int, 1>>);
        static_assert(!detail::has_constexpr_size<std::vector<int>>);

        /*if constexpr (detail::has_compile_time_sized_container<Containers...>::value) {
            static_assert(detail::has_compile_time_sized_container<Containers...>::size(containers) > 0);
        } else*/ {
            std::optional<std::size_t> firstSize{};
            bool result = true;
            detail::for_each_in_tuple(containers, [&](const auto &c) {
                if (firstSize)
                    result &= *firstSize == c.size();
                else
                    firstSize = c.size();
            });
            if (!result)
                throw std::runtime_error("invalid container sizes");
        }

    }
public:
    struct DontCheckSizes {};
    constexpr ContainerContainer(DontCheckSizes, Containers&... containers): containers(containers...) {
        sizeCheck(this->containers);
    }
    constexpr ContainerContainer(Containers&... containers): containers(containers...) {
        sizeCheck(this->containers);
    }

    struct IteratorContainer {
        struct BeginType {};
        struct EndType {};
        IteratorContainer(BeginType, std::tuple<Containers&...>& containers):
        it(std::apply([](Containers&... c) { return IteratorContainer::make_begin_iterator(c...); }, containers)) {}
        IteratorContainer(EndType, std::tuple<Containers&...>& containers):
        it(std::apply([](Containers&... c) { return IteratorContainer::make_end_iterator(c...); }, containers)) {}
        static auto make_begin_iterator(Containers&... containers) {
            return std::make_tuple(containers.begin()...);
        }
        static auto make_end_iterator(Containers&... containers) {
            return std::make_tuple(containers.end()...);
        }
        decltype(make_begin_iterator(std::declval<Containers&>()...)) it;
        IteratorContainer& operator++() {
            detail::for_each_in_tuple(it, [](auto &i) { ++i; });
            return *this;
        }
        bool operator==(const IteratorContainer& _rhs) const {
            return it == _rhs.it;
        }
        bool operator!=(const IteratorContainer& _rhs) const {
            return it != _rhs.it;
        }
        auto operator*() {
            return std::apply([](auto&... i) { return std::tie(*i...); }, it);
        }
    };
    struct ConstIteratorContainer {
        struct BeginType {};
        struct EndType {};
        ConstIteratorContainer(BeginType, const std::tuple<const Containers&...>& containers):
                it(std::apply([](const Containers&... c) { return ConstIteratorContainer::make_begin_iterator(c...); }, containers)) {}
        ConstIteratorContainer(EndType, const std::tuple<const Containers&...>& containers):
                it(std::apply([](const Containers&... c) { return ConstIteratorContainer::make_end_iterator(c...); }, containers)) {}
        static auto make_begin_iterator(const Containers&... containers) {
            return std::make_tuple(containers.begin()...);
        }
        static auto make_end_iterator(const Containers&... containers) {
            return std::make_tuple(containers.end()...);
        }
        decltype(make_begin_iterator(std::declval<Containers const&>()...)) it;
        ConstIteratorContainer& operator++() {
            detail::for_each_in_tuple(it, [](auto &i) { ++i; });
            return *this;
        }
        bool operator==(const ConstIteratorContainer& _rhs) const {
            return it == _rhs.it;
        }
        bool operator!=(const ConstIteratorContainer& _rhs) const {
            return it != _rhs.it;
        }
        auto operator*() const {
            return std::apply([](const auto&... i) { return std::tie(*i...); }, it);
        }
    };

    ConstIteratorContainer begin() const {
        return {typename ConstIteratorContainer::BeginType{}, containers};
    }
    ConstIteratorContainer end() const {
        return {typename ConstIteratorContainer::EndType{}, containers};
    }
    IteratorContainer begin() {
        return IteratorContainer{typename IteratorContainer::BeginType{}, containers};
    }
    IteratorContainer end() {
        return {typename IteratorContainer::EndType{}, containers};
    }


private:
    std::tuple<Containers&...> containers;

};

}
