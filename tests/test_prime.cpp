#include <catch2/catch.hpp>
#include <units/primes.hpp>


TEST_CASE("Prime factorization of some number", "[prime]")
{
    auto res = units::prime_factorization(2 * 2 * 7 * 13 * 17 * 17);
    REQUIRE(res.size == 4);
    CHECK(res.factors[0].factor == 2);
    CHECK(res.factors[0].exponent == 2);
    CHECK(res.factors[1].factor == 7);
    CHECK(res.factors[1].exponent == 1);
    CHECK(res.factors[2].factor == 13);
    CHECK(res.factors[2].exponent == 1);
    CHECK(res.factors[3].factor == 17);
    CHECK(res.factors[3].exponent == 2);
}


TEST_CASE("Prime factorization of big prime", "[prime]")
{
    uint64_t big_prime = 5'053'038'781'909'696'477;
    auto res = units::prime_factorization(big_prime);
    REQUIRE(res.size == 1);
    CHECK(res.factors[0].factor == big_prime);
    CHECK(res.factors[0].exponent == 1);
}


TEST_CASE("Prime factorization of power of 2", "[prime]")
{
    auto res = units::prime_factorization(1ULL << 63u);
    REQUIRE(res.size == 1);
    CHECK(res.factors[0].factor == 2);
    CHECK(res.factors[0].exponent == 63);
}