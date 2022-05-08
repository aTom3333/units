#include "unit_definition.h"

#include <catch2/catch.hpp>
#include <units/quantity.hpp>


TEST_CASE("quantity to and from scalar", "[quantity]")
{
    {
        units::quantity<metre> distance = 0.5 * m;
        CHECK(distance.in<metre>() == 0.5);
        CHECK(distance.in(m) == 0.5);
    }

    {
        // implicitly convertible to/from scalar
        units::quantity<units::ScalarUnit> value = 1.5;
        double res = value;
        CHECK(res == 1.5);
    }
}


TEST_CASE("Unit conversion", "[quantity]")
{
    units::quantity<metre> distance = 0.5 * m;
    CHECK(distance.in<kilometre>() == Approx(0.0005));
    CHECK(distance.in(mm) == Approx(500));
}


TEST_CASE("Operation on quantities", "[quantity]")
{
    units::quantity<metre> distance = 2 * m;
    distance += 3 * m;
    distance -= 1 * m;
    CHECK(distance.in(m) == 4);
    CHECK(distance == 4 * m);
    CHECK(distance < 5 * m);
    CHECK(distance <= 5 * m);
    CHECK(distance != 5 * m);
    CHECK(distance > 3 * m);
    CHECK(distance >= 3 * m);

    units::quantity<metre_per_second> speed = distance / (2 * s);
    CHECK(speed == 2 * m / s);

    auto distance10 = speed * 10 * s;
    auto distance10_2 = speed * (10 * s);
    CHECK(distance10 == 20 * m);
    CHECK(distance10 == distance10_2);
}