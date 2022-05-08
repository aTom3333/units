#ifndef MAGNITUDE_HPP
#define MAGNITUDE_HPP

#include <cstdint>
#include "power.hpp"
#include "primes.hpp"
#include "type_name.hpp"


namespace units
{
    template<uint64_t N>
    struct int_factor
    {
        static constexpr int64_t value = N;
    };

    struct pi
    {
        template<typename T>
        static constexpr T value()
        {
            return T(3.141592653589793238462643383279502884197169399375105820974944L);
        }
    };

    namespace detail
    {
        /**
         * Represent a positive real number at compile time
         * The representation used is a list a list of factor raised to some power,
         * each factor can either be a prime integer or a special type that represents
         * an irrational number symbolically.
         * 
         * The list is sorted by factor so that 2 magnitudes that represent the same 
         * value are identical types.
         * 
         * This class template should not be instantiated directly by client code
         * because invariants need to be maintained.
         */
        template<typename... FactorPowers>
        struct magnitude_raw {};

        std::false_type is_magnitude_impl(...);
        template<typename... FactorPowers>
        std::true_type is_magnitude_impl(const magnitude_raw<FactorPowers...>&);

        template<typename T>
        inline constexpr bool is_magnitude = decltype(is_magnitude_impl(std::declval<T>()))::value;

        template<uint64_t N>
        struct magnitude_from_int_impl
        {
            static_assert(N > 0, "Only positive integer supported");

            static constexpr auto factorization = prime_factorization(N);

            template<size_t... I>
            static constexpr auto extract_factors(std::index_sequence<I...>)
            {
                return meta::make_typelist(meta::type<Power<
                    int_factor<factorization.factors[I].factor>,
                    factorization.factors[I].exponent>>...);
            }

            static constexpr auto make()
            {
                // factors are already sorted
                auto factors = extract_factors(std::make_index_sequence<factorization.size>{});
                return factors.template as_type<magnitude_raw>();
            }
        };

        template<typename T>
        std::false_type is_int_factor_impl(const T&);
        template<uint64_t N>
        std::true_type is_int_factor_impl(const int_factor<N>&);

        template<typename T>
        inline constexpr bool is_int_factor = decltype(is_int_factor_impl(std::declval<T>()))::value;


        struct MagnitudeComparator
        {
            template<typename PowFactorType1, typename PowFactorType2>
            constexpr auto
            operator()(const PowFactorType1&,
                       const PowFactorType2&) const
            {
                using PowFactor1 = typename PowFactorType1::type;
                using PowFactor2 = typename PowFactorType2::type;
                using Factor1 = typename PowFactor1::Base;
                using Factor2 = typename PowFactor2::Base;
                // Compare int factor by value
                // Compare other factors by type name
                // Other factor are always after int factor
                if constexpr(is_int_factor<Factor1> && is_int_factor<Factor2>)
                {
                    constexpr auto val1 = Factor1::value;
                    constexpr auto val2 = Factor2::value;
                    if constexpr(val1 == val2)
                        return meta::val<meta::ordering::equivalent>;
                    else if constexpr(val1 < val2)
                        return meta::val<meta::ordering::less>;
                    else
                        return meta::val<meta::ordering::greater>;
                }
                else if constexpr(is_int_factor<Factor1>)
                    return meta::val<meta::ordering::less>;
                else if constexpr(is_int_factor<Factor2>)
                    return meta::val<meta::ordering::greater>;
                else
                {
                    constexpr auto factor1Name = meta::type_name<Factor1>();
                    constexpr auto factor2Name = meta::type_name<Factor2>();
                    if constexpr(factor1Name == factor2Name)
                        return meta::val<meta::ordering::equivalent>;
                    else if constexpr(factor1Name < factor2Name)
                        return meta::val<meta::ordering::less>;
                    else
                        return meta::val<meta::ordering::greater>;
                }
            }
        };

        template<typename... FactorPowers>
        constexpr auto magnitude_as_typelist_impl(const magnitude_raw<FactorPowers...>&)
        {
            return meta::make_typelist(meta::type<FactorPowers>...);
        }

        template<typename Magnitude>
        constexpr auto magnitude_as_typelist()
        {
            return decltype(magnitude_as_typelist_impl(std::declval<Magnitude>())){};
        }

        template<typename Magnitude>
        constexpr auto inverse_magnitude_impl()
        {
            auto factor_list = magnitude_as_typelist<Magnitude>();
            auto inverse_list = meta::transform(factor_list, [](auto powerType)
            {
                using FactorPower = typename decltype(powerType)::type;
                using Factor = typename FactorPower::Base;
                return meta::type<Power<Factor, -FactorPower::exponent>>;
            });
            return inverse_list.template as_type<magnitude_raw>();
        }

        template<typename Magnitude1, typename Magnitude2>
        constexpr auto multiply_magnitude_impl()
        {
            auto factor_list1 = magnitude_as_typelist<Magnitude1>();
            auto factor_list2 = magnitude_as_typelist<Magnitude2>();
            auto result_list = meta::merge_combine_filter(factor_list1, factor_list2,
                                                          MagnitudeComparator{}, PowerCombiner{});
            return result_list.template as_type<magnitude_raw>();
        }
    }

    /**
     * Create a magnitude from an integer
     */
    template<uint64_t N>
    using MagnitudeFromInt = typename decltype(detail::magnitude_from_int_impl<N>::make())::type;

    /**
     * Create a magnitude that represent the inverse of the given magnitude
     */
    template<typename Magnitude>
    using InverseMagnitude = typename decltype(detail::inverse_magnitude_impl<Magnitude>())::type;

    /**
     * Create a magnitude that represents the product of the two given magnitudes
     */
    template<typename Magnitude1, typename Magnitude2>
    using MultiplyMagnitude =
    typename decltype(detail::multiply_magnitude_impl<Magnitude1, Magnitude2>())::type;

    /**
     * Create a magnitude from a ratio
     */
    template<typename Ratio>
    using MagnitudeFromRatio = MultiplyMagnitude<MagnitudeFromInt<Ratio::num>,
        InverseMagnitude<MagnitudeFromInt<Ratio::den>>>;

    /**
     * Create a magnitude that represents an irrational number symbolically
     */
    template<typename IrrationalFactor>
    using MagnitudeFromIrrational = detail::magnitude_raw<Power<IrrationalFactor, 1>>;
}

#endif // MAGNITUDE_HPP