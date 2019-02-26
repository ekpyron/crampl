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
}

template<typename... Containers>
class ContainerContainer {
    static void sizeCheck(std::tuple<Containers&...> containers) {
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
public:
    ContainerContainer(Containers&... containers): containers(containers...) {
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
