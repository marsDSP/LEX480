#pragma once

#ifndef LEX480_TYPE_LIST_H
#define LEX480_TYPE_LIST_H

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace MarsDSP::Utils::inline types_list
{
    template<typename TypesList, typename ForEachAction, size_t typeIndex = 0, size_t numTypesLeft = TypesList::count>
    static constexpr void forEach(ForEachAction &&forEachAction)
    {
        if constexpr (numTypesLeft > 0)
        {
            forEachAction(std::integral_constant<std::size_t, typeIndex>());
            forEach<TypesList, ForEachAction, typeIndex + 1, numTypesLeft - 1>(
                std::forward<ForEachAction>(forEachAction));
        }
    }

    namespace detail
    {
        template<typename TypesList, typename OtherListType, size_t typeIndex = 0, size_t numTypesLeft =
                OtherListType::count>
        struct ConcatHelper
        {
            using ConcatenatedListType = ConcatHelper<typename TypesList::template InsertBack<
                    typename OtherListType::template AtIndex<typeIndex> >,
                OtherListType,
                typeIndex + 1,
                numTypesLeft - 1>::ConcatenatedListType;
        };

        template<typename TypesList, typename OtherListType, size_t typeIndex>
        struct ConcatHelper<TypesList, OtherListType, typeIndex, 0>
        {
            using ConcatenatedListType = TypesList;
        };

        template<typename TypesList, typename TupleType, size_t typeIndex = 0, size_t numTypesLeft = std::tuple_size<
            TupleType>()>
        struct TupleHelpers
        {
            using TupleTypesList = TupleHelpers<typename TypesList::template InsertBack<std::tuple_element_t<
                    typeIndex, TupleType> >,
                TupleType,
                typeIndex + 1,
                numTypesLeft - 1>::TupleTypesList;
        };

        template<typename TypesList, typename TupleType, size_t typeIndex>
        struct TupleHelpers<TypesList, TupleType, typeIndex, 0>
        {
            using TupleTypesList = TypesList;
        };

        template<typename TypesList, typename Type>
        static constexpr int indexOf()
        {
            int result = -1;
            forEach<TypesList>(
                [&result](auto typeIndex)
                {
                    using TypeAtIndex = TypesList::template AtIndex<typeIndex>;
                    if constexpr (std::is_same_v<Type, TypeAtIndex>)
                        result = static_cast<int>(typeIndex);
                });
            return result;
        }
    }


    template<typename... Ts>
    struct TypesList
    {
        using Types = std::tuple<Ts...>;


        static constexpr auto count = std::tuple_size_v<Types>;


        template<size_t I>
        using AtIndex = std::tuple_element_t<I, Types>;


        template<typename T>
        static constexpr int IndexOf = detail::indexOf<TypesList, T>();


        template<typename T>
        static constexpr bool contains = (std::is_same_v<T, Ts> || ...);


        template<typename... NewTs>
        using InsertFront = TypesList<NewTs..., Ts...>;


        template<typename... NewTs>
        using InsertBack = TypesList<Ts..., NewTs...>;


        template<typename OtherListType>
        using Concatenate = detail::ConcatHelper<TypesList, OtherListType>::ConcatenatedListType;
    };


    template<typename TupleType>
    using TupleList = detail::TupleHelpers<TypesList<>, TupleType>::TupleTypesList;

    namespace detail
    {
        template<typename MaybeTypesList>
        struct IsTypesListHelper
        {
            static constexpr bool value = false;
        };

        template<typename... Ts>
        struct IsTypesListHelper<TypesList<Ts...> >
        {
            static constexpr bool value = true;
        };
    }


    template<typename MaybeTypesList>
    constexpr bool IsTypesList = detail::IsTypesListHelper<MaybeTypesList>::value;

    namespace detail
    {
        template<typename T>
        struct TypesWrapperHelper
        {
            using Types = T;
        };

        template<typename... Ts>
        struct TypesWrapperHelper<TypesList<Ts...> >
        {
            using Types = TypesList<Ts...>::Types;
        };
    }


    template<typename T>
    using TypesWrapper = detail::TypesWrapperHelper<T>::Types;
}
#endif
