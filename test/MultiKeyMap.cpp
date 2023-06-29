/**
 *
 *
 * @file MultiKeyMap.cpp
 * @brief 
 * @author clonker
 * @date 2/1/19
 */

#include <map>

#include <catch2/catch_all.hpp>

#include "crampl/MultiKeyMap.h"

using Example = crampl::MultiKeyMap<std::map, crampl::ComplexKey<int, std::greater<int>>, int, double>;
using Example2 = crampl::MultiKeyMap<std::map, int, double, float, int, double>;
using Xeample2 = std::map<int, std::map<double, std::map<float, std::map<int, double>>>>;

TEST_CASE("MultiKeyMap") {
    REQUIRE(std::is_same_v<std::map<int, std::map<int, double>, std::greater<int>>, Example>);
    REQUIRE(std::is_same_v<Xeample2, Example2>);
    REQUIRE(std::is_same_v<std::map<int, bool>, crampl::MultiKeyMap<std::map, int, bool>>);

    std::map<std::string, int> m4 {{"foo", 3}};
    REQUIRE(!crampl::find(m4, "bar"));
    REQUIRE(crampl::find(m4, "foo"));

    REQUIRE(crampl::emplace(m4, std::piecewise_construct, std::forward_as_tuple(std::string("x")), std::forward_as_tuple(42)) == 42);
    int x = 21;
    REQUIRE(m4["x"] == 42);
    REQUIRE(crampl::emplace(m4, std::piecewise_construct, std::forward_as_tuple(std::string("y")), std::forward_as_tuple(x)) == 21);
    REQUIRE(m4["y"] == 21);

    std::map<std::string, std::map<int, float>> m3_2 {{"foo", {{3, 3.0f}}}};
    crampl::emplace(m3_2, std::piecewise_construct, std::forward_as_tuple(std::string("x")), std::forward_as_tuple(7), std::forward_as_tuple(4.0f));


    crampl::MultiKeyMap<std::map, int, int, int> m3 { {21, { {42, 84} }} };
    REQUIRE(crampl::find(m3, 21, 42) == std::optional<int>{84});


    {
        crampl::MultiKeyMap<std::map, int, std::string, std::vector<char>> m4 { {21, { {"ABC", {{true},{false}}} }} };

        auto& three = crampl::emplace(m4, std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple("a"), std::forward_as_tuple(std::vector<char>{{false},{true}}));
        REQUIRE(three.size() == 2);
        REQUIRE(three[0] == false);
        REQUIRE(three[1] == true);
    }
    {
        auto six = crampl::emplace(m4, std::piecewise_construct, std::forward_as_tuple("baz"),
                                   std::forward_as_tuple(6));
        REQUIRE(six == 6);
        REQUIRE(crampl::find(m4, "baz") == std::optional<int>{6});
        auto alsoSix = crampl::emplace(m4, std::piecewise_construct, std::forward_as_tuple("baz"),
                                       std::forward_as_tuple(7));
        REQUIRE(alsoSix == 6);
    }
    {
        auto& three = crampl::emplace(m3, std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple(2), std::forward_as_tuple(3));
        REQUIRE(three == 3);
        auto alsoThree = crampl::emplace(m3, std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple(2), std::forward_as_tuple(789));
        REQUIRE(alsoThree == 3);
        REQUIRE(crampl::find(m3, 1, 2) == 3);
        three = 4;
        REQUIRE(crampl::find(m3, 1, 2) == 4);
    }

}
