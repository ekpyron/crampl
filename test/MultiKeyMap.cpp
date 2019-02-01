/**
 *
 *
 * @file MultiKeyMap.cpp
 * @brief 
 * @author clonker
 * @date 2/1/19
 */

#include <map>

#include <catch2/catch.hpp>

#include "crampl/MultiKeyMap.h"

using Example = crampl::MultiKeyMap<std::map, crampl::ComplexKey<int, std::greater<int>>, int, double>;

using Example2 = crampl::MultiKeyMap<std::map, int, double, float, int, double>;
using Xeample2 = std::map<int, std::map<double, std::map<float, std::map<int, double>>>>;

TEST_CASE("MultiKeyMap") {
    REQUIRE(std::is_same_v<std::map<int, std::map<int, double>, std::greater<int>>, Example>);
    REQUIRE(std::is_same_v<Xeample2, Example2>);
}
