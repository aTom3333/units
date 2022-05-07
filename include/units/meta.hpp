#ifndef META_HPP
#define META_HPP

#include <type_traits>
#include <utility>
#include <compare>


namespace meta
{
    enum struct ordering : int
    {
        less = -1, equivalent = 0, greater = 1
    };

    inline constexpr bool is_less_or_equivalent(ordering ord)
    {
        return static_cast<int>(ord) <= 0;
    }

    template<auto v>
    struct ValueConstant
    {
        static constexpr auto value = v;

        constexpr operator decltype(value)() const { return value; }

        constexpr auto operator!() const { return ValueConstant<!v>{}; }
    };

    #define BIN_OP(op) \
    template<auto l, auto r> \
    constexpr auto operator op (const ValueConstant<l>&, const ValueConstant<r>&) \
    { return ValueConstant<(l op r)>{}; }

    BIN_OP(+)

    BIN_OP(-)

    BIN_OP(*)

    BIN_OP(/)

    BIN_OP(==)

    BIN_OP(!=)

    BIN_OP(<)

    BIN_OP(>)

    BIN_OP(<=)

    BIN_OP(>=)

    #undef BIN_OP

    template<auto v>
    inline constexpr ValueConstant<v> val{};


    template<typename T>
    struct TypeConstant
    {
        using type = T;
    };

    template<typename L, typename R>
    constexpr auto operator==(const TypeConstant<L>&, const TypeConstant<R>&)
    {
        return val<std::is_same_v<L, R>>;
    }

    template<typename L, typename R>
    constexpr auto operator!=(const TypeConstant<L>& l, const TypeConstant<R>& r)
    {
        return !(l == r);
    }

    template<typename T>
    inline constexpr TypeConstant<T> type{};

    template<typename T>
    constexpr auto type_of(const T&)
    {
        return type<T>;
    }

    struct NoTypeConstant : TypeConstant<NoTypeConstant> {};
    inline constexpr NoTypeConstant notype;


    #define meta_if(...) if constexpr(decltype(__VA_ARGS__)::value)

    template<typename... Ts>
    struct typelist
    {
        static constexpr auto size() { return val<sizeof...(Ts)>; }

        static constexpr auto empty() { return size() == val<0>; }

        template<typename T>
        constexpr auto append(const TypeConstant<T>& elem) const
        {
            return typelist<Ts..., T>{};
        }

        template<typename T>
        constexpr auto prepend(const TypeConstant<T>& elem) const
        {
            return typelist<T, Ts...>{};
        }

        template<typename Head, typename... Tail>
        struct helper_impl
        {
            using head = Head;
            using tail = typelist<Tail...>;
        };

        template<typename U = std::void_t<Ts...>, typename = std::enable_if_t<!empty(), U>>
        constexpr auto head() const
        {
            using helper = helper_impl<Ts...>;
            return type<typename helper::head>;
        }

        template<typename U = std::void_t<Ts...>, typename = std::enable_if_t<!empty(), U>>
        auto tail() const
        {
            using helper = helper_impl<Ts...>;
            return typename helper::tail{};
        }

        template<template<typename...> class Tmp>
        using as = Tmp<Ts...>;

        template<template<typename...> class Tmp>
        constexpr auto as_type() const
        {
            return type<as<Tmp>>;
        }
    };

    template<typename... Ts>
    constexpr auto make_typelist(const TypeConstant<Ts>& ...)
    {
        return typelist<Ts...>{};
    }

    template<typename>
    struct is_typelist : std::false_type {};

    template<typename... Ts>
    struct is_typelist<typelist<Ts...>> : std::true_type {};

    template<auto... Vs>
    using valuelist = typelist<ValueConstant<Vs>...>;

    template<typename... Ts1, typename... Ts2>
    constexpr auto concat(const typelist<Ts1...>&, const typelist<Ts2...>&)
    {
        return typelist<Ts1..., Ts2...>{};
    }

    template<typename Idx, typename LeftList, typename RightList>
    constexpr auto split_impl(const Idx& idx, const LeftList& left, const RightList& right)
    {
        meta_if(idx > val<0>)
        {
            // move elements from right to left
            auto newOutput1 = left.append(right.head());
            return split_impl(idx - val<1>, newOutput1, right.tail());
        }
        else
            return std::make_pair(left, right);
    }

    template<typename Idx, typename List>
    constexpr auto split(const Idx& idx, const List& list)
    {
        return split_impl(idx, typelist<>{}, list);
    }

    template<typename ResultList, typename SortedList1, typename SortedList2, typename Comparator>
    constexpr auto
    merge_impl(const ResultList& result, const SortedList1& list1,
               const SortedList2& list2, const Comparator& Comp)
    {
        meta_if(list1.empty())return concat(result, list2);
        else meta_if(list2.empty())return concat(result, list1);
        else
        {
            constexpr auto compare_result = val<decltype(Comp(list1.head(), list2.head()))::value>;
            if constexpr(is_less_or_equivalent(compare_result))
            {
                auto newResult = result.append(list1.head());
                auto newList1 = list1.tail();
                return merge_impl(newResult, newList1, list2, Comp);
            }
            else
            {
                auto newResult = result.append(list2.head());
                auto newList2 = list2.tail();
                return merge_impl(newResult, list1, newList2, Comp);
            }
        }
    }

    template<typename SortedList1, typename SortedList2, typename Comparator>
    constexpr auto merge(const SortedList1& list1, const SortedList2& list2, const Comparator& Comp)
    {
        return merge_impl(typelist<>{}, list1, list2, Comp);
    }

    template<typename... Ts, typename Comparator>
    constexpr auto sort(const typelist<Ts...>& list, const Comparator& Comp)
    {
        meta_if(list.size() < val<2>)return list;

        auto half_size = list.size() / val<2>;
        auto[left, right] = split(half_size, list);
        auto sorted_left = sort(left, Comp);
        auto sorted_right = sort(right, Comp);
        return merge(sorted_left, sorted_right, Comp);
    }


    template<typename... Ts, typename Fn>
    constexpr auto transform(const typelist<Ts...>&, const Fn& fn)
    {
        return typelist<typename decltype(fn(type<Ts>))::type...>{};
    }

    template<typename ResultList, typename SortedList1, typename SortedList2, typename Comparator, typename Combine>
    constexpr auto
    merge_combine_filter_impl(const ResultList& result, const SortedList1& list1, const SortedList2& list2,
                              const Comparator& Comp, const Combine& combine)
    {
        meta_if(list1.empty())return concat(result, list2);
        else meta_if(list2.empty())return concat(result, list1);
        else
        {
            constexpr auto compare_result = val<decltype(Comp(list1.head(), list2.head()))::value>;
            if constexpr(compare_result == ordering::equivalent)
            {
                // Need to combine
                auto newList1 = list1.tail();
                auto newList2 = list2.tail();
                auto combine_result = combine(list1.head(), list2.head());
                meta_if(combine_result == type_of(notype))
                {
                    // filtered out, add nothing to result
                    return merge_combine_filter_impl(result, newList1, newList2, Comp, combine);
                }
                else
                {
                    // Add the reduced thing to result 
                    auto newResult = result.append(combine_result);
                    return merge_combine_filter_impl(newResult, newList1, newList2, Comp, combine);
                }
            }
            else if constexpr(compare_result == ordering::less)
            {
                // Move from list 1 to result
                auto newResult = result.append(list1.head());
                auto newList1 = list1.tail();
                return merge_combine_filter_impl(newResult, newList1, list2, Comp, combine);
            }
            else
            {
                // Move from list 2 to result
                auto newResult = result.append(list2.head());
                auto newList2 = list2.tail();
                return merge_combine_filter_impl(newResult, list1, newList2, Comp, combine);
            }
        }
    }

    template<typename SortedList1, typename SortedList2, typename Comparator, typename Combine>
    constexpr auto merge_combine_filter(const SortedList1& list1, const SortedList2& list2,
                                        const Comparator& Comp, const Combine& combine)
    {
        return merge_combine_filter_impl(typelist<>{}, list1, list2, Comp, combine);
    }

    template<typename Result, typename InputList, typename Reducer>
    constexpr auto reduce(const Result& result, const InputList& list, const Reducer& reducer)
    {
        meta_if(list.empty())return result;
        else
            return reduce(reducer(result, list.head()), list.tail(), reducer);
    }

    #undef meta_if
}

#endif // META_HPP