#ifndef DOWNCAST_HPP
#define DOWNCAST_HPP

#include <type_traits>


namespace meta
{
    template<typename BaseType>
    struct downcast_base
    {
        using downcast_base_type = BaseType;
        friend auto downcast_guide(downcast_base);
    };

    template<typename Target, typename T>
    struct downcast_child : T
    {
        friend auto downcast_guide(typename downcast_child::downcast_base) { return Target(); }
    };

    namespace detail
    {
        template<typename T, typename = void>
        struct has_downcast : std::false_type {};
        template<typename T>
        struct has_downcast<T, std::void_t<decltype(
        downcast_guide(std::declval<downcast_base<T>>())
        )>> : std::true_type
        {
        };

        template<typename T>
        constexpr auto downcast_target_impl()
        {
            if constexpr(has_downcast<T>::value)
                return decltype(downcast_guide(std::declval<downcast_base<T>>()))();
            else
                return T();
        }
    }

    template<typename T>
    using downcast = decltype(detail::downcast_target_impl<T>());
}

#endif // DOWNCAST_HPP