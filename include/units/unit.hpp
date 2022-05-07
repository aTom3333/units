#ifndef UNIT_HPP
#define UNIT_HPP

#include "dimension.hpp"
#include "magnitude.hpp"


namespace units
{
    namespace detail
    {
        template<typename Dim, typename Mag>
        struct unit_raw : meta::downcast_base<unit_raw<Dim, Mag>>
        {
            using Dimension = Dim;
            using Magnitude = Mag;
            using Raw = unit_raw;
        };

        template<typename Child, typename UnitRaw>
        struct named_unit : meta::downcast_child<Child, UnitRaw> {};

        std::false_type is_unit_impl(...);
        template<typename Dim, typename Mag>
        std::true_type is_unit_impl(const unit_raw<Dim, Mag>&);

        template<typename T>
        inline constexpr bool is_unit = decltype(is_unit_impl(std::declval<T>()))::value;

        template<typename UnitRaw>
        using inverse_unit_raw = unit_raw<
            inverse_dimension_raw < typename UnitRaw::Dimension>, InverseMagnitude<typename UnitRaw::Magnitude>>;

        template<typename Unit>
        using inverse_unit = meta::downcast<inverse_unit_raw<Unit>>;

        template<typename UnitRaw1, typename UnitRaw2>
        using multiply_unit_raw = unit_raw<
            meta::downcast<
                make_combined_dimension_raw <
                Power < typename UnitRaw1::Dimension, 1>,
            Power < typename UnitRaw2::Dimension, 1>
        >
        >,
        MultiplyMagnitude<typename UnitRaw1::Magnitude, typename UnitRaw2::Magnitude>
        >;

        template<typename Unit1, typename Unit2>
        using multiply_unit = meta::downcast<multiply_unit_raw<Unit1, Unit2>>;

        template<typename Unit1, typename Unit2, typename = std::enable_if_t<
            is_unit<std::decay_t<Unit1>> && is_unit<std::decay_t<Unit2>>>>
        constexpr auto operator*(const Unit1&, const Unit2&)
        {
            return multiply_unit<Unit1, Unit2>{};
        }

        template<typename Unit1, typename Unit2, typename = std::enable_if_t<
            is_unit<std::decay_t<Unit1>> && is_unit<std::decay_t<Unit2>>>>
        constexpr auto operator/(const Unit1&, const Unit2&)
        {
            return multiply_unit<Unit1, inverse_unit<Unit2>>{};
        }
    }

    /**
     * Defines a base unit for a given dimension.
     * Usage is as follow (supposing that a base dimension Length has been defined):
     * `struct metre : BaseUnit<Length> {};`
     */
    template<typename Child, typename Dimension>
    using BaseUnit = detail::named_unit<Child, detail::unit_raw<Dimension, detail::magnitude_raw<>>>;

    /**
     * Define a scaled version of the given unit.
     * Usage is as follow (supposing that a metre unit has been defined):
     * `struct kilometre : ScaledUnit<kilometre, metre, MagnitudeFromInt<1000>> {};`
     */
    template<typename Child, typename Unit, typename Magnitude>
    using ScaledUnit = detail::named_unit<Child, detail::unit_raw<typename Unit::Dimension,
        MultiplyMagnitude < typename Unit::Magnitude, Magnitude>>>;


    struct ScalarUnit : BaseUnit<ScalarUnit, Scalar> {};
}

#endif // UNIT_HPP