#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include "power.hpp"
#include "downcast.hpp"
#include "meta.hpp"
#include "type_name.hpp"


namespace units
{
    namespace detail
    {
        /**
         * The effective unnamed type that represents a dimension
         * It is made up of a (sorted) list of base dimensions raised to some power
         * 
         * Each type in the list should be an instantiation of Power<Dim, exp>
         * where Dim is a base dimension, ie a type that inherits from Dimension
         * 
         * The list is sorted by the base dimension types, this is done so that
         * dimensions that are comprised of the same base dimensions raised to the
         * same powers are of the same type.
         * 
         * This class template should not be instantiated directly by client code
         * because invariants need to be maintained.
         */
        template<typename... DimensionPowers>
        struct dimension_raw : meta::downcast_base<dimension_raw<DimensionPowers...>>
        {
            static constexpr auto typelist() { return meta::typelist<DimensionPowers...>{}; };
        };

        /**
         * This class does nothing more than inheriting from dimension_raw while
         * hooking into the downcast protocol.
         * 
         * This let one register name for a certain dimension.
         * For example, supposing that the base dimension Length and Time are defined
         * `struct Acceleration : named_dimension<Acceleration, Power<Length, 1>, Power<Time, -2>> {};`
         * Now the Acceleration name for this dimension is registered
         * When later the type `dimension_raw<Power<Length, 1>, Power<Time, -2>>`
         * appears somewhere (most likely as a result of a generic operation)
         * and is passed through the downcast mechanism, it will be replaced by the
         * type `Acceleration` newly defined.
         * (because `Acceleration` publicly inherits `dimension_raw<Power<Length, 1>, Power<Time, -2>>`
         * the code expecting a dimension_raw still works as expected but the clearer
         * `Acceleration` name will appear in error message).
         * 
         * This class template should not be instantiated directly by client code
         * because invariants need to be maintained. Use the helper aliases instead.
         */
        template<typename Child, typename DimensionRaw>
        struct named_dimension : meta::downcast_child<Child, DimensionRaw>
        {
            using Raw = DimensionRaw;
        };
    }

    /**
     * Helper to create a base dimension.
     * Usage is as follow:
     * `struct Length : BaseDimension<Length> {};`
     * Length is now a new base dimension
     */
    template<typename Child>
    using BaseDimension = detail::named_dimension<Child, detail::dimension_raw<Power<Child, 1>>>;

    namespace detail
    {
        struct DimensionComparator
        {
            template<typename PowDimType1, typename PowDimType2>
            constexpr auto operator()(const PowDimType1&,
                                      const PowDimType2&) const
            {
                using PowDim1 = typename PowDimType1::type;
                using PowDim2 = typename PowDimType2::type;
                using Dim1 = typename PowDim1::Base;
                using Dim2 = typename PowDim2::Base;
                // Compare the dimension types by there type names
                constexpr auto dim1Name = meta::type_name<Dim1>();
                constexpr auto dim2Name = meta::type_name<Dim2>();
                if constexpr(dim1Name == dim2Name)
                    return meta::val<meta::ordering::equivalent>;
                else if constexpr(dim1Name < dim2Name)
                    return meta::val<meta::ordering::less>;
                else
                    return meta::val<meta::ordering::greater>;
            }
        };

        template<typename DimensionPower>
        constexpr auto to_base_dimensions()
        {
            constexpr int exp_to_multiply = DimensionPower::exponent;
            using dim_raw = typename DimensionPower::Base;
            auto base_dimensions = dim_raw::typelist();
            auto pow_multiplier = [](auto dim_power_type)
            {
                using DimPower = typename decltype(dim_power_type)::type;
                using BaseDim = typename DimPower::Base;
                constexpr auto exp = DimPower::exponent;
                static_assert(exp * exp_to_multiply != 0);
                using NewDimPower = Power<BaseDim, exp * exp_to_multiply>;
                return meta::type<NewDimPower>;
            };
            return meta::transform(base_dimensions, pow_multiplier);
        }

        template<typename... DimensionPowers>
        constexpr auto make_combined_dimension_raw_impl()
        {
            // Some of the given dimension might already be composite, so we need to
            // replace their usage by the list of base dimension they are made of
            // and then we need to correctly combine the list of list of base dimension (raised to some power).
            auto list_of_list_of_dim_power = meta::make_typelist(
                meta::type_of(to_base_dimensions<DimensionPowers>())...);
            auto reducer = [](auto accumulated_list, auto list_to_add_type)
            {
                return meta::merge_combine_filter(accumulated_list,
                                                  typename decltype(list_to_add_type)::type{},
                                                  DimensionComparator{}, PowerCombiner{});
            };
            return meta::reduce(meta::make_typelist(), list_of_list_of_dim_power,
                                reducer).template as_type<dimension_raw>();
        }

        template<typename... DimensionPowers>
        using make_combined_dimension_raw =
        typename decltype(make_combined_dimension_raw_impl<DimensionPowers...>())::type;

        template<typename Dimension>
        constexpr auto inverse_dimension_raw_impl()
        {
            auto result_list = meta::transform(Dimension::typelist(), [](auto powerType)
            {
                using PowerType = typename decltype(powerType)::type;
                using InversePower = typename PowerType::inverse;
                return meta::type<InversePower>;
            });
            return result_list.template as_type<dimension_raw>();
        }

        template<typename Dimension>
        using inverse_dimension_raw = typename decltype(inverse_dimension_raw_impl<Dimension>())::type;
    }


    /**
     * Helper to create a combined dimension.
     * Usage is as follow (supposing that the `Length` and `Time` base dimension already exist):
     * `struct Acceleration : CombinedDimension<Acceleration, Power<Length, 1>, Power<Time, -2>> {};`
     * 
     * The dimensions used inside the `Power` don't need to be base dimension, they can also
     * be combined dimension themselves. For example:
     * `struct Speed : CombinedDimension<Speed, Power<Acceleration, 1>, Power<Time, 1>> {};`
     * is equivalent to
     * `struct Speed : CombinedDimension<Speed, Power<Length, 1>, Power<Time, -1>> {};`
     */
    template<typename Child, typename... DimensionPowers>
    using CombinedDimension = detail::named_dimension<Child,
        detail::make_combined_dimension_raw<DimensionPowers...>>;


    struct Scalar : detail::named_dimension<Scalar, detail::dimension_raw<>> {};
}

#endif // DIMENSION_HPP