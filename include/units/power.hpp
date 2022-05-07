#ifndef CORE_HPP
#define CORE_HPP

#include "meta.hpp"


namespace units
{
    /**
     * Represents *something* raised to a power. 
     * What the something is depends on usage.
     * 
     * For example it can be used as Power<std::integral_constant<int, 2>, 3>
     * to represent the value 2^3
     * or it can be used as Power<Length, 2> to represent the area dimension
     * (the length dimension squared)
     */
    template<typename T, int N>
    struct Power
    {
        static_assert(N != 0, "Exponent of 0 is forbidden");

        using Base = T;
        static constexpr int exponent = N;

        using inverse = Power<T, -exponent>;
    };

    namespace detail
    {
        struct PowerCombiner
        {
            template<typename Pow1Type, typename Pow2Type>
            constexpr auto operator()(const Pow1Type&, const Pow2Type&) const
            {
                using Pow1 = typename Pow1Type::type;
                using Pow2 = typename Pow2Type::type;
                using Base = typename Pow1::Base;
                static_assert(meta::type<Base> == meta::type<typename Pow2::Base>);
                constexpr auto exp1 = Pow1::exponent;
                constexpr auto exp2 = Pow2::exponent;
                if constexpr (exp1 + exp2 == 0)
                    return meta::notype;
                else
                    return meta::type<Power<Base, exp1 + exp2>>;
            }
        };
    }
}

#endif // CORE_HPP