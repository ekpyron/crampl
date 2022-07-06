/**
 *
 *
 * @file ContainerContainer.cpp
 * @brief 
 * @author clonker
 * @date 2/8/19
 */
#include <catch2/catch_all.hpp>

#include "crampl/ContainerContainer.h"

TEST_CASE("ContainerContainer const") {
    auto list1 = {1, 2};
    auto list2 = {1.2, 2.2};
    for (auto [i1,i2]: crampl::ContainerContainer(list1, list2))
    {
        CHECK(i1 == decltype(i1)(i2));

        static_assert(std::is_same_v<decltype(i1), std::decay_t<decltype(*list1.begin())> const&>);
        static_assert(std::is_same_v<decltype(i2), std::decay_t<decltype(*list2.begin())> const&>);
    }
}

TEST_CASE("ContainerContainer const container") {
    std::vector<int> list1 {1, 2};
    std::vector<double>  list2 {1.2, 2.2};
    [](const std::vector<int> &list1, const std::vector<double> &list2) {
        for (auto [i1,i2]: crampl::ContainerContainer(list1, list2))
        {
            CHECK(i1 == decltype(i1)(i2));

            static_assert(std::is_same_v<decltype(i1), std::decay_t<decltype(*list1.begin())> const&>);
            static_assert(std::is_same_v<decltype(i2), std::decay_t<decltype(*list2.begin())> const&>);
        }
    }(list1, list2);
}

TEST_CASE("ContainerContainer const container container") {
    std::vector<int> list1 {1, 2};
    std::vector<double>  list2 {1.2, 2.2};
    crampl::ContainerContainer containerContainer(list1, list2);
    [](const auto& containerContainer) {
        for (auto [i1,i2]: containerContainer)
        {
            CHECK(i1 == decltype(i1)(i2));

            static_assert(std::is_same_v<decltype(i1), std::decay_t<decltype(*list1.begin())> const&>);
            static_assert(std::is_same_v<decltype(i2), std::decay_t<decltype(*list2.begin())> const&>);
        }
    }(containerContainer);
}


TEST_CASE("ContainerContainer non-const") {
    std::vector<int> list1 {1, 2};
    auto list2 = {1.2, 2.2};
    auto cc = crampl::ContainerContainer(list1, list2);
    for (auto [i1,i2]: cc)
    {
        CHECK(i1 == std::decay_t<decltype(i1)>(i2));

        static_assert(std::is_same_v<decltype(i1), std::decay_t<decltype(*list1.begin())>&>);
        static_assert(std::is_same_v<decltype(i2), std::decay_t<decltype(*list2.begin())>const &>);

        i1 = 0;
    }
    for (auto const& [i]: crampl::ContainerContainer(list1)) REQUIRE(i == 0);
}

TEST_CASE("ContainerContainer SizeCheck") {
    std::vector<int> list1 {1, 2, 3};
    std::vector<double> list2 {1.2, 2.2};
    REQUIRE_THROWS(crampl::ContainerContainer(list1, list2));
}
