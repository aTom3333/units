#ifndef QUANTITY_HPP
#define QUANTITY_HPP

#include "unit.hpp"


namespace units
{
    struct ApplyMagnitudeAsFloat;

    template<typename Unit, typename T = double, typename ApplyMagnitudePolicy = ApplyMagnitudeAsFloat>
    class quantity;

    namespace detail
    {
        struct quantity_maker;
        
        std::false_type is_quantity_impl(...);
        template<typename Unit, typename T, typename Policy>
        std::true_type is_quantity_impl(const quantity<Unit, T, Policy>&);
        
        template<typename T>
        inline constexpr bool is_quantity = decltype(is_quantity_impl(std::declval<T>()))::value; 

        template<typename Unit, typename T, typename ApplyMagnitudePolicy = ApplyMagnitudeAsFloat>
        class quantity_base
        {
            using Quantity = quantity<Unit, T, ApplyMagnitudePolicy>;

            constexpr Quantity& that() { return *static_cast<Quantity*>(this); }

            constexpr const Quantity& that() const { return *static_cast<const Quantity*>(this); }

        public:
            using value_type = T;
            using unit = Unit;

            template<typename Unit2, typename = std::enable_if_t<
                std::is_same_v<typename Unit::Dimension, typename Unit2::Dimension> && detail::is_unit<Unit2>>>
            constexpr quantity<Unit2, T, ApplyMagnitudePolicy> as(Unit2 = {}) const;

            template<typename Unit2, typename = std::enable_if_t<
                std::is_same_v<typename Unit::Dimension, typename Unit2::Dimension> && detail::is_unit<Unit2>>>
            constexpr T in(Unit2 = {}) const;

            constexpr Quantity operator+() const { return Quantity(+value_); }

            constexpr Quantity operator-() const { return Quantity(-value_); }

            constexpr Quantity& operator+=(const Quantity& other)
            {
                value_ += other.value_;
                return that();
            }

            constexpr Quantity& operator-=(const Quantity& other)
            {
                value_ -= other.value_;
                return that();
            }

            constexpr Quantity& operator*=(const quantity<ScalarUnit, T, ApplyMagnitudePolicy>& other)
            {
                value_ *= other.value_;
                return that();
            }

            constexpr Quantity& operator/=(const quantity<ScalarUnit, T, ApplyMagnitudePolicy>& other)
            {
                value_ /= other.value_;
                return that();
            }
            
            constexpr bool operator==(const Quantity& other) const
            {
                return value_ == other.value_;
            }
            
            constexpr bool operator!=(const Quantity& other) const
            {
                return value_ != other.value_;
            }
            
            constexpr bool operator<(const Quantity& other) const
            {
                return value_ < other.value_;
            }
            
            constexpr bool operator<=(const Quantity& other) const
            {
                return value_ <= other.value_;
            }
            
            constexpr bool operator>(const Quantity& other) const
            {
                return value_ > other.value_;
            }
            
            constexpr bool operator>=(const Quantity& other) const
            {
                return value_ >= other.value_;
            }
            
            template<typename OtherT, typename = std::enable_if_t<std::is_convertible_v<T, OtherT>>>
            operator quantity<Unit, OtherT, ApplyMagnitudePolicy>() const;

        protected:
            template<typename Unit2, typename T2, typename ApplyMagnitudePolicy2>
            friend class quantity_base;

            constexpr explicit quantity_base(T value) : value_{std::move(value)} {}

            T value_;
        };
    }

    template<typename Unit, typename T, typename ApplyMagnitudePolicy>
    class quantity : public detail::quantity_base<Unit, T, ApplyMagnitudePolicy>
    {
        using base = detail::quantity_base<Unit, T, ApplyMagnitudePolicy>;
        using base::base;

        friend struct detail::quantity_maker;
        
        template<typename Unit2, typename T2, typename ApplyMagnitudePolicy2>
        friend class quantity_base;

    public:
        quantity(Unit) : base(1) {}
    };

    // Special case for scalar unit which is to treated as a scalar
    // Implicit conversion to and from T
    template<typename T, typename ApplyMagnitudePolicy>
    class quantity<ScalarUnit, T, ApplyMagnitudePolicy>
        : public detail::quantity_base<ScalarUnit, T, ApplyMagnitudePolicy>
    {
        using base = detail::quantity_base<ScalarUnit, T, ApplyMagnitudePolicy>;
        using base::base;

        friend struct detail::quantity_maker;
        
        template<typename Unit2, typename T2, typename ApplyMagnitudePolicy2>
        friend class quantity_base;

    public:
        quantity(ScalarUnit) : base(1) {}

        quantity(T value) : base(std::move(value)) {}

        operator T() const { return this->value_; }
    };

    namespace detail
    {
        template<int exp, typename T>
        constexpr auto int_pow(const T& value)
        {
            if constexpr(exp < 0)
                return 1 / int_pow<-exp>(value);
            if constexpr(exp == 0)
                return T(1);
            else if constexpr(exp == 1)
                return value;
            else if constexpr(exp % 2 == 0)
                return int_pow<exp / 2>(value * value);
            else
                return int_pow<exp / 2>(value * value) * value;
        }
    }

    struct ApplyMagnitudeAsFloat
    {
        template<typename Magnitude, typename T>
        static constexpr auto apply(const T& value)
        {
            // using float or more precise type
            using AccumulationType = decltype(std::declval<T>() * std::declval<float>());
            const AccumulationType factor =
                meta::reduce(AccumulationType(1),
                             detail::magnitude_as_typelist<Magnitude>(),
                             [](const AccumulationType& acc, auto factorType)
                             {
                                 using FactorPower = typename decltype(factorType)::type;
                                 using Factor = typename FactorPower::Base;
                                 if constexpr(detail::is_int_factor<Factor>)
                                     return acc * detail::int_pow<FactorPower::exponent>(
                                         AccumulationType(Factor::value));
                                 else
                                     return acc * detail::int_pow<FactorPower::exponent>(
                                         Factor::template value<AccumulationType>());
                             });

            return factor * AccumulationType(value);
        }
    };

    namespace detail
    {
        struct quantity_maker
        {
            template<typename Quantity>
            static constexpr Quantity make(typename Quantity::value_type value)
            {
                return Quantity(std::move(value));
            }

            template<typename Quantity>
            static constexpr Quantity add(const Quantity& lhs, const Quantity& rhs)
            {
                return Quantity(lhs.value_ + rhs.value_);
            }

            template<typename Quantity>
            static constexpr Quantity sub(const Quantity& lhs, const Quantity& rhs)
            {
                return Quantity(lhs.value_ - rhs.value_);
            }

            template<typename QuantityRes, typename QuantityLHS, typename QuantityRHS>
            static constexpr QuantityRes times(const QuantityLHS& lhs, const QuantityRHS& rhs)
            {
                return QuantityRes(lhs.value_ * rhs.value_);
            }

            template<typename QuantityRes, typename QuantityLHS, typename QuantityRHS>
            static constexpr QuantityRes divide(const QuantityLHS& lhs, const QuantityRHS& rhs)
            {
                return QuantityRes(lhs.value_ / rhs.value_);
            }
        };

        template<typename Unit, typename T, typename ApplyMagnitudePolicy>
        template<typename Unit2, typename>
        constexpr T quantity_base<Unit, T, ApplyMagnitudePolicy>::in(Unit2) const
        {
            using MagnitudeToApply = MultiplyMagnitude<typename Unit::Magnitude, InverseMagnitude<typename Unit2::Magnitude>>;
            return T(ApplyMagnitudePolicy::template apply<MagnitudeToApply>(value_));
        }

        template<typename Unit, typename T, typename ApplyMagnitudePolicy>
        template<typename Unit2, typename>
        constexpr quantity<Unit2, T, ApplyMagnitudePolicy> quantity_base<Unit, T, ApplyMagnitudePolicy>::as(Unit2) const
        {
            return quantity_maker::make<quantity<Unit2, T, ApplyMagnitudePolicy>>(in<Unit2>());
        }
        template<typename Unit, typename T, typename ApplyMagnitudePolicy>
        template<typename OtherT, typename>
        quantity_base<Unit, T, ApplyMagnitudePolicy>::operator quantity<
            Unit, OtherT, ApplyMagnitudePolicy>() const
        {
            return quantity_maker::make<quantity<Unit, OtherT, ApplyMagnitudePolicy>>(OtherT(value_));
        }
    }

    template<typename Unit, typename T, typename ApplyMagnitudePolicy>
    constexpr auto
    operator+(const quantity<Unit, T, ApplyMagnitudePolicy>& lhs, const quantity<Unit, T, ApplyMagnitudePolicy>& rhs)
    {
        return detail::quantity_maker::add<quantity<Unit, T, ApplyMagnitudePolicy>>(lhs, rhs);
    }

    template<typename Unit, typename T, typename ApplyMagnitudePolicy>
    constexpr auto
    operator-(const quantity<Unit, T, ApplyMagnitudePolicy>& lhs, const quantity<Unit, T, ApplyMagnitudePolicy>& rhs)
    {
        return detail::quantity_maker::sub<quantity<Unit, T, ApplyMagnitudePolicy>>(lhs, rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename Unit2, typename T2>
    constexpr auto operator*(const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs,
                             const quantity<Unit2, T2, ApplyMagnitudePolicy>& rhs)
    {
        using ResUnit = detail::multiply_unit<Unit1, Unit2>;
        using ResT = decltype(std::declval<T1>() * std::declval<T2>());
        return detail::quantity_maker::times<quantity<ResUnit, ResT, ApplyMagnitudePolicy>>(lhs, rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename T2, typename = std::enable_if_t<!detail::is_unit<T2> && !detail::is_quantity<T2>>>
    constexpr auto operator*(const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs,
                             const T2& rhs)
    {
        return lhs * quantity<ScalarUnit, T2, ApplyMagnitudePolicy>(rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename T2, typename = std::enable_if_t<!detail::is_unit<T2> && !detail::is_quantity<T2>>>
    constexpr auto operator*(const T2& rhs, const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs)
    {
        return lhs * quantity<ScalarUnit, T2, ApplyMagnitudePolicy>(rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename T2, typename = std::enable_if_t<!detail::is_unit<T2> && !detail::is_quantity<T2>>>
    constexpr auto operator/(const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs,
                             const T2& rhs)
    {
        return lhs / quantity<ScalarUnit, T2, ApplyMagnitudePolicy>(rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename T2, typename = std::enable_if_t<!detail::is_unit<T2> && !detail::is_quantity<T2>>>
    constexpr auto operator/(const T2& rhs, const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs)
    {
        return lhs / quantity<ScalarUnit, T2, ApplyMagnitudePolicy>(rhs);
    }

    template<typename Unit1, typename T1, typename ApplyMagnitudePolicy, typename Unit2, typename T2>
    constexpr auto operator/(const quantity<Unit1, T1, ApplyMagnitudePolicy>& lhs,
                             const quantity<Unit2, T2, ApplyMagnitudePolicy>& rhs)
    {
        using ResUnit = detail::multiply_unit<Unit1, detail::inverse_unit<Unit2>>;
        using ResT = decltype(std::declval<T1>() / std::declval<T2>());
        return detail::quantity_maker::divide<quantity<ResUnit, ResT, ApplyMagnitudePolicy>>(lhs, rhs);
    }
}

// Multiply/divide scalar against unit
template<typename Unit, typename T, typename = std::enable_if_t<
    units::detail::is_unit<Unit> && !units::detail::is_unit<T> && !units::detail::is_quantity<T>>>
constexpr auto operator*(T&& value, Unit)
{
    return units::detail::quantity_maker::make<units::quantity<Unit, T, units::ApplyMagnitudeAsFloat>>(
        std::forward<T>(value));
}

template<typename Unit, typename T, typename = std::enable_if_t<
    units::detail::is_unit<Unit> && !units::detail::is_unit<T> && !units::detail::is_quantity<T>>>
constexpr auto operator*(Unit, T&& value)
{
    return units::detail::quantity_maker::make<units::quantity<Unit, T, units::ApplyMagnitudeAsFloat>>(
        std::forward<T>(value));
}

template<typename Unit, typename T, typename = std::enable_if_t<
    units::detail::is_unit<Unit> && !units::detail::is_unit<T> && !units::detail::is_quantity<T>>>
constexpr auto operator/(T&& value, Unit)
{
    return units::detail::quantity_maker::make<units::quantity<units::detail::inverse_unit<Unit>, T, units::ApplyMagnitudeAsFloat>>(
        std::forward<T>(value));
}

template<typename Unit, typename T, typename = std::enable_if_t<
    units::detail::is_unit<Unit> && !units::detail::is_unit<T> && !units::detail::is_quantity<T>>>
constexpr auto operator/(Unit, T&& value)
{
    return units::detail::quantity_maker::make<units::quantity<Unit, T, units::ApplyMagnitudeAsFloat>>(
        std::forward<T>(1 / value));
}

// Multiply/divide quantity against unit
template<typename Unit1, typename T, typename ApplyMagnitudePolicy, typename Unit2, typename = std::enable_if_t<
    units::detail::is_unit<Unit2>>>
constexpr auto operator*(const units::quantity<Unit1, T, ApplyMagnitudePolicy>& value, Unit2)
{
    return units::detail::quantity_maker::make<units::quantity<units::detail::multiply_unit<Unit1, Unit2>, T, units::ApplyMagnitudeAsFloat>>(
        value.template in<Unit1>());
}

template<typename Unit1, typename T, typename ApplyMagnitudePolicy, typename Unit2, typename = std::enable_if_t<
    units::detail::is_unit<Unit2>>>
constexpr auto operator*(Unit2, const units::quantity<Unit1, T, ApplyMagnitudePolicy>& value)
{
    return units::detail::quantity_maker::make<units::quantity<units::detail::multiply_unit<Unit1, Unit2>, T, units::ApplyMagnitudeAsFloat>>(
        value.template in<Unit1>());
}


template<typename Unit1, typename T, typename ApplyMagnitudePolicy, typename Unit2, typename = std::enable_if_t<
                                                                                        units::detail::is_unit<Unit2>>>
constexpr auto operator/(const units::quantity<Unit1, T, ApplyMagnitudePolicy>& value, Unit2)
{
    return units::detail::quantity_maker::make<units::quantity<units::detail::multiply_unit<Unit1, units::detail::inverse_unit<Unit2>>, T, units::ApplyMagnitudeAsFloat>>(
        value.template in<Unit1>());
}


template<typename Unit1, typename T, typename ApplyMagnitudePolicy, typename Unit2, typename = std::enable_if_t<
                                                                                        units::detail::is_unit<Unit2>>>
constexpr auto operator/(Unit2, const units::quantity<Unit1, T, ApplyMagnitudePolicy>& value)
{
    return units::detail::quantity_maker::make<units::quantity<units::detail::multiply_unit<units::detail::inverse_unit<Unit1>, Unit2>, T, units::ApplyMagnitudeAsFloat>>(
        T(1) / value.template in<Unit1>());
}

#endif // QUANTITY_HPP