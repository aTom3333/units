#ifndef PRIMES_HPP
#define PRIMES_HPP

#include <array>
#include <cstdint>
#include <cstddef>


namespace units
{
    struct prime_factorization_result_t
    {
        struct factor_exponent
        {
            uint64_t factor;
            int exponent;
        };
        // 14 is enough for the prime factorization of every integer smaller than 2^64
        // because the smaller integer with 15 factors in its prime factorization
        // is the product fo the 15 first prime numbers
        // (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23 * 29 * 31 * 37 * 41 * 43 * 47 * 53)
        // which is 32 589 158 477 190 044 730 and is greater than 
        // 2^64 = 18 446 744 073 709 551 616
        std::array<factor_exponent, 14> factors{};
        size_t size = 0;

        constexpr void add_factor(uint64_t factor, int exponent)
        {
            factors[size] = {factor, exponent};
            ++size;
        }
    };

    constexpr prime_factorization_result_t prime_factorization(uint64_t n)
    {
        prime_factorization_result_t result;
        const uint64_t originalN = n;

        // n must be divisible by factor for calling this function
        auto process_factor = [&](uint64_t factor)
        {
            // check to what power n is divisible by factor and add to the prime factorization
            int exponent = 0;
            do
            {
                n /= factor;
                ++exponent;
            } while(n % factor == 0);
            result.add_factor(factor, exponent);
        };


        if(n % 2 == 0)
            process_factor(2);
        if(n % 3 == 0)
            process_factor(3);

        uint64_t factor = 1;
        uint64_t incr = 4;

        auto advance_to_next_prime = [&]
        {
            // Advance to next prime that divides n
            // note: no need to check that factor is prime,
            // if it divides n, it must be prime because every composite number
            // smaller than factor are product of primes number that already been
            // handled
            do
            {
                factor += incr;
                incr ^= 6ULL; // toggle between 2 and 4 to have a 6-wheel
            } while(n % factor != 0 && factor * factor < originalN);
        };

        advance_to_next_prime();

        while(n > 1 && factor * factor < originalN)
        {
            process_factor(factor);
            advance_to_next_prime();
        }

        if(n > 1)
        {
            // If n has not been reduced to 1, it means n is greater than
            // sqrt(originalN) and it divides originalN so it must be prime
            result.add_factor(n, 1);
        }


        return result;
    }
}

#endif // PRIMES_HPP