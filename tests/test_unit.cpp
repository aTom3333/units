#include "unit_definition.h"

#include <catch2/catch.hpp>


TEST_CASE("Units combine correctly", "[unit]")
{
    CHECK(std::is_same_v<decltype(m / s * s), metre>);
    CHECK(std::is_same_v<decltype(m / s), metre_per_second>);
    CHECK(std::is_same_v<decltype(km / ks), metre_per_second>);
    CHECK(std::is_same_v<decltype(mm / ms), metre_per_second>);
    CHECK(std::is_same_v<decltype(km * ms), decltype(mm * ks)>);
}