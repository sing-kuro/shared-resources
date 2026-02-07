/**
 * Copyright (c) 2026 Kuro Amami
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

///
/// @file shared_resources.hpp
///

#ifndef SHARED_RESOURCES_SHARED_RESOURCES_HPP
#define SHARED_RESOURCES_SHARED_RESOURCES_HPP

#include <concepts>
#include <functional>
#include <type_traits>

namespace srs
{
namespace internals
{

///
/// @brief Checks for duplicate types in a parameter pack
/// @tparam Types The types to check for duplicates
/// @note Inherits from std::true_type if no duplicates are found, otherwise std::false_type
///
template <typename... Types>
struct has_no_duplicates;

template <>
struct has_no_duplicates<>
    : public std::true_type
{
};

template <typename T>
struct has_no_duplicates<T>
    : public std::true_type
{
};

template <typename Head, typename Second, typename... Tail>
struct has_no_duplicates<Head, Second, Tail...>
    : public std::conditional_t<!std::is_same_v<Head, Second> && has_no_duplicates<Head, Tail...>::value && has_no_duplicates<Second, Tail...>::value,
                                std::true_type,
                                std::false_type>

{
};

/// @brief Concept to ensure no duplicate types in a parameter pack
template <typename... Types>
concept no_duplicates = has_no_duplicates<Types...>::value;

}  // namespace internals


///
/// @brief A compile-time list of types with no duplicates
/// @tparam Types The types in the list
///
template <typename... Types>
    requires internals::no_duplicates<Types...>
struct type_list
{
    template <typename T>
    using prepend = type_list<T, Types...>;
};

///
/// @brief Trait to check if a type is a type_list
/// @tparam T The type to check
/// @note Inherits from std::true_type if T is a type_list, otherwise std::false_type
///
template <typename T>
struct is_type_list
    : public std::false_type
{
};

template <typename... Types>
struct is_type_list<type_list<Types...>>
    : public std::true_type
{
};

/// @brief Concept to ensure a type is a type_list
template <typename T>
concept type_list_concept = is_type_list<T>::value;

template <type_list_concept List, typename... Exclude>
class shared_resources;

///
/// @brief Trait to check if a type is a shared_resources
/// @tparam T The type to check
/// @note Inherits from std::true_type if T is a shared_resources, otherwise std::false_type
///
template <typename T>
struct is_shared_resources
    : public std::false_type
{
};

template <type_list_concept List, typename... Exclude>
struct is_shared_resources<shared_resources<List, Exclude...>>
    : public std::true_type
{
};

/// @brief Concept to ensure a type is a shared_resources
template <typename T>
concept shared_resources_concept = is_shared_resources<T>::value;

template <type_list_concept List, typename... Exclude>
class shared_references;

///
/// @brief Trait to check if a type is a shared_references
/// @tparam T The type to check
/// @note Inherits from std::true_type if T is a shared_references, otherwise std::false_type
///
template <typename T>
struct is_shared_references
    : public std::false_type
{
};

template <type_list_concept List, typename... Exclude>
struct is_shared_references<shared_references<List, Exclude...>>
    : public std::true_type
{
};

/// @brief Concept to ensure a type is a shared_references
template <typename T>
concept shared_references_concept = is_shared_references<T>::value;

/// @brief Concept to ensure a type is either a shared_resources or shared_references
template <typename T>
concept shared_concept = shared_resources_concept<T> || shared_references_concept<T>;

namespace internals
{

///
/// @brief Wraps each type in a type_list with std::reference_wrapper
///
template <type_list_concept List>
struct wrap_with_reference;

template <typename... Types>
struct wrap_with_reference<type_list<Types...>>
{
    using type = type_list<std::reference_wrapper<Types>...>;
};

///
/// @brief Removes specified types from a type_list
/// @tparam List The original type_list
/// @tparam Exclude The types to remove from the type_list
///
template <type_list_concept List, typename... Exclude>
struct remove_types;

template <type_list_concept List>
struct remove_types<List>
{
    using type = List;
};

template <typename Exclude>
struct remove_types<type_list<>, Exclude>
{
    using type = type_list<>;
};

template <typename Exclude, typename Head, typename... Tail>
struct remove_types<type_list<Head, Tail...>, Exclude>
{
    using type = std::conditional_t<std::is_same_v<Head, Exclude>,
                                    typename remove_types<type_list<Tail...>, Exclude>::type,
                                    typename remove_types<type_list<Tail...>, Exclude>::type::template prepend<Head>>;
};

template <type_list_concept List, typename Head, typename... Tail>
struct remove_types<List, Head, Tail...>
{
    using type = typename remove_types<typename remove_types<List, Head>::type, Tail...>::type;
};

///
/// @brief Checks if type T is contained in type_list U
///
template <typename T, type_list_concept U>
struct contains;

template <typename T>
struct contains<T, type_list<>>
    : public std::false_type
{
};

template <typename T, typename Head, typename... Tail>
struct contains<T, type_list<Head, Tail...>>
    : public std::conditional_t<std::is_same_v<T, Head> || contains<T, type_list<Tail...>>::value,
                                std::true_type, std::false_type>
{
};

template <typename T, typename U>
concept contains_concept = type_list_concept<U> && contains<T, U>::value;

///
/// @brief Checks if all types in T are contained in U
///
template <type_list_concept T, type_list_concept U>
struct contains_all;

template <type_list_concept U>
struct contains_all<type_list<>, U>
    : public std::true_type
{
};

template <type_list_concept U, typename Head, typename... Tail>
struct contains_all<type_list<Head, Tail...>, U>
    : public std::conditional_t<contains<Head, U>::value && contains_all<type_list<Tail...>, U>::value,
                                std::true_type, std::false_type>
{
};

/// @brief Concept to ensure all types in type_list T are contained in type_list U
template <typename T, typename U>
concept contains_all_concept = type_list_concept<T> && type_list_concept<U> && contains_all<T, U>::value;

///
/// @brief Base case for getting the first argument of type Target
/// @tparam Target The type to search for
/// @param head The only argument
/// @return The argument of type Target
///
template <typename Target>
Target get(Target head)
{
    return head;
}

///
/// @brief Gets the first argument of type Target from a variadic list of arguments
/// @tparam Target The type to search for
/// @tparam Head The type of the first argument
/// @tparam Tail The types of the remaining arguments
/// @param head The first argument
/// @param tail The remaining arguments
/// @return The first argument of type Target
///
template <typename Target, typename Head, typename... Tail>
    requires contains_concept<Target, type_list<Head, Tail...>>
Target get(Head head, Tail... tail)
{
    if constexpr (std::is_same_v<Target, Head>)
    {
        return head;
    }
    else
    {
        return get<Target>(tail...);
    }
}

///
/// @brief Storage for shared resources
/// @tparam List A type_list of resource types to store
///
template <type_list_concept List>
class storage;

template <>
class storage<type_list<>>
{
};

template <typename T>
class storage<type_list<T>>
{
public:
    ///
    /// @brief Default constructor
    ///
    constexpr storage() noexcept = default;

    ///
    /// @brief Constructs storage with the given arguments
    /// @tparam Args The types of the arguments
    /// @param args The arguments to construct the storage
    ///
    template <typename... Args>
        requires contains_concept<T, type_list<Args...>>
    constexpr storage(Args... args) noexcept
        : data_(internals::get<T>(args...))
    {
    }

    ///
    /// @brief Constructs storage by combining two storages
    /// @tparam ListA The type_list of the first storage
    /// @tparam ListB The type_list of the second storage
    /// @param a The first storage
    /// @param b The second storage
    ///
    template <type_list_concept ListA, type_list_concept ListB>
        requires contains_concept<T, ListA> || contains_concept<T, ListB>
    constexpr storage(storage<ListA> const &a, storage<ListB> const &b) noexcept
        : data_(get_head(a, b))
    {
    }

    // TODO: Consider adding rvalue reference constructor

    ///
    /// @brief Gets a reference to the stored resource
    /// @tparam U The type of the resource to get
    /// @return A reference to the stored resource
    ///
    template <typename U>
        requires std::same_as<T, U>
    constexpr U &get() noexcept
    {
        return data_;
    }

    ///
    /// @brief Gets a const reference to the stored resource
    /// @tparam U The type of the resource to get
    /// @return A const reference to the stored resource
    ///
    template <typename U>
        requires std::same_as<T, U>
    constexpr U const &get() const noexcept
    {
        return data_;
    }

private:
    ///
    /// @brief Gets the stored resource of type T from either storage a or b
    /// @tparam ListA The type_list of storage a
    /// @tparam ListB The type_list of storage b
    /// @param a The first storage
    /// @param b The second storage
    /// @return The stored resource of type T
    ///
    template <type_list_concept ListA, type_list_concept ListB>
    constexpr static T get_head(storage<ListA> const &a, storage<ListB> const &b) noexcept
    {
        if constexpr (contains<T, ListA>::value)
        {
            return a.template get<T>();
        }
        else
        {
            return b.template get<T>();
        }
    }

    /// @brief The stored resource
    T data_;
};

template <typename Head, typename... Tail>
class storage<type_list<Head, Tail...>>
{
private:
public:
    ///
    /// @brief Default constructor
    ///
    constexpr storage() noexcept = default;

    ///
    /// @brief Constructs storage from another storage
    /// tparam OtherTypes The types in the other storage
    /// @param other The other storage to copy from
    ///
    template <typename... OtherTypes>
        requires contains_all_concept<type_list<Head, Tail...>, type_list<OtherTypes...>>
    constexpr storage(storage<type_list<OtherTypes...>> const &other)
        : data_(other.template get<Head>()), rest_(other)
    {
    }

    ///
    /// @brief Constructs storage with the given arguments
    /// @tparam Args The types of the arguments
    /// @param args The arguments to construct the storage
    ///
    template <typename... Args>
        requires contains_all_concept<type_list<Head, Tail...>, type_list<Args...>>
    constexpr storage(Args const &...args) noexcept
        : data_(internals::template get<Head>(args...)), rest_(args...)
    {
    }

    ///
    /// @brief Constructs storage by combining two storages
    /// @tparam ListA The type_list of the first storage
    /// @tparam ListB The type_list of the second storage
    /// @param a The first storage
    /// @param b The second storage
    ///
    template <type_list_concept ListA, type_list_concept ListB>
        requires contains_concept<Head, ListA> || contains_concept<Head, ListB>
    constexpr storage(storage<ListA> const &a, storage<ListB> const &b) noexcept
        : data_(get_head(a, b)), rest_(a, b)
    {
    }

    // TODO: Consider adding rvalue reference constructor

    ///
    /// @brief Gets a reference to the stored resource of type U
    /// @tparam U The type of the resource to get
    /// @return A reference to the stored resource of type U
    ///
    template <typename U>
    constexpr U &get() noexcept
    {
        if constexpr (std::is_same_v<Head, U>)
        {
            return data_;
        }
        else
        {
            return rest_.template get<U>();
        }
    }

    ///
    /// @brief Gets a const reference to the stored resource of type U
    /// @tparam U The type of the resource to get
    /// @return A const reference to the stored resource of type U
    ///
    template <typename U>
    constexpr U const &get() const noexcept
    {
        if constexpr (std::is_same_v<Head, U>)
        {
            return data_;
        }
        else
        {
            return rest_.template get<U>();
        }
    }

private:
    ///
    /// @brief Gets the stored resource of type Head from either storage a or b
    /// @tparam ListA The type_list of storage a
    /// @tparam ListB The type_list of storage b
    /// @param a The first storage
    /// @param b The second storage
    /// @return The stored resource of type Head
    ///
    template <type_list_concept ListA, type_list_concept ListB>
    constexpr static Head get_head(storage<ListA> const &a, storage<ListB> const &b) noexcept
    {
        if constexpr (contains<Head, ListA>::value)
        {
            return a.template get<Head>();
        }
        else
        {
            return b.template get<Head>();
        }
    }

    /// @brief The stored resource of type Head
    Head data_;

    /// @brief The storage for the remaining types
    storage<type_list<Tail...>> rest_;
};

///
/// @brief Trait to check if a type is a storage
/// @tparam T The type to check
///
template <typename T>
struct is_storage
    : public std::false_type
{
};

template <type_list_concept T>
struct is_storage<storage<T>>
    : public std::true_type
{
};

template <typename T>
concept storage_concept = is_storage<T>::value;

}  // internals

///
/// @brief Provides shared resources of specified types, excluding certain types
/// @tparam List A type_list of resource types to share
/// @tparam Exclude A type_list of resource types to exclude from sharing
///
template <type_list_concept List, typename... Exclude>
class shared_resources
{
private:
    /// @brief The list of types after excluding specified types
    using list = typename internals::remove_types<List, Exclude...>::type;

public:
    ///
    /// @brief Default constructor
    ///
    constexpr shared_resources() noexcept = default;

    ///
    /// @brief Constructs shared_resources with the given arguments
    /// @tparam Args The types of the arguments
    /// @param args The arguments to construct the shared resources
    ///
    template <typename... Args>
        requires internals::contains_all_concept<list, type_list<Args...>>
    constexpr shared_resources(Args... args) noexcept
        : data_(args...)
    {
    }

    ///
    /// @brief Default copy constructor
    /// @param other The other shared_resources to copy from
    ///
    constexpr shared_resources(shared_resources const &other) noexcept = default;

    ///
    /// @brief Default move constructor
    /// @param other The other shared_resources to move from
    ///
    constexpr shared_resources(shared_resources &&other) noexcept = default;

    ///
    /// @brief Constructs shared_resources from another shared_resources with the same effective type list
    /// @tparam OtherList The type_list of the other shared_resources
    /// @tparam OtherExclude The types to exclude from the other shared_resources
    /// @param other The other shared_resources to copy from
    ///
    template <type_list_concept OtherList, typename... OtherExclude>
        requires std::same_as<list, typename internals::remove_types<OtherList, OtherExclude...>::type>
    constexpr shared_resources(shared_resources<OtherList, OtherExclude...> const &other) noexcept
        : data_(other.data_)
    {
    }

    ///
    /// @brief Constructs shared_resources from another shared_resources with additional arguments
    /// @tparam Other The type of the other shared_resources
    /// @tparam Args The types of the additional arguments
    /// @param other The other shared_resources to copy from
    /// @param args Additional arguments to construct the shared resources
    ///
    template <shared_resources_concept Other, typename... Args>
    constexpr shared_resources(Other const &other, Args const &...args)
        : data_(create_storage(other.data_, args...))
    {
    }

    ///
    /// @brief Gets a reference to the shared resource of type U
    /// @tparam U The type of the resource to get
    /// @return A reference to the shared resource of type U
    ///
    template <typename U>
    constexpr U &get() noexcept
    {
        return data_.template get<U>();
    }

    ///
    /// @brief Gets a const reference to the shared resource of type U
    /// @tparam U The type of the resource to get
    /// @return A const reference to the shared resource of type U
    ///
    template <typename U>
    constexpr U const &get() const noexcept
    {
        return data_.template get<U>();
    }

private:
    /// @brief The storage type for the shared resources
    using storage_type = internals::storage<list>;

    /// @brief The storage for the shared resources
    storage_type data_;

    ///
    /// @brief Creates storage from another shared_resources and additional arguments
    /// @tparam OtherTypeList The type_list of the other shared_resources
    /// @tparam Args The types of the additional arguments
    /// @param other The other shared_resources to copy from
    /// @param args Additional arguments to construct the shared resources
    /// @return The constructed storage
    ///
    template <internals::storage_concept OtherType, typename... Args>
    constexpr static storage_type create_storage(OtherType const &other, Args const &...args)
    {
        internals::storage<type_list<Args...>> add(args...);
        return storage_type(other, add);
    }

    /// @brief Allow shared_resources to access private members
    template <type_list_concept, typename...>
    friend class shared_resources;
};

namespace internals
{
///
/// @brief Helper to create a shared_resources type from a type_list and an exclude type_list
/// @tparam List The original type_list
/// @tparam Exclude The types to exclude from the type_list
///
template <type_list_concept List, type_list_concept Exclude>
struct shared_resources_exclude_list;

template <type_list_concept List, typename... Exclude>
struct shared_resources_exclude_list<List, type_list<Exclude...>>
{
    using type = shared_resources<List, Exclude...>;
};

}

///
/// @brief Provides shared references to resources of specified types, excluding certain types
/// @tparam List A type_list of resource types to share references for
/// @tparam Exclude A parameter pack of resource types to exclude from sharing
///
template <type_list_concept List, typename... Exclude>
class shared_references
{
private:
    /// @brief The list of types after excluding specified types
    using list = typename internals::remove_types<List, Exclude...>::type;

public:
    ///
    /// @brief Constructs shared_references with the given arguments
    /// @tparam Args The types of the arguments
    /// @param args The arguments to construct the shared references
    ///
    template <typename... Args>
        requires internals::contains_all_concept<list, type_list<Args...>>
    constexpr shared_references(Args &...args) noexcept
        : data_(std::reference_wrapper(args)...)
    {
    }

    ///
    /// @brief Constructs shared_references from another shared_references and additional arguments
    /// @tparam Other The type of the other shared_references
    /// @tparam Args The types of the additional arguments
    /// @param other The other shared_references to copy from
    /// @param args Additional arguments to construct the shared references
    ///
    template <shared_references_concept Other, typename... Args>
    constexpr shared_references(Other const &other, Args &...args) noexcept
        : data_(other.data_, std::reference_wrapper(args)...)
    {
    }

    ///
    /// @brief Gets a reference to the shared resource of type U
    /// @tparam U The type of the resource to get
    /// @return A reference to the shared resource of type U
    ///
    template <typename U>
    U &get() const noexcept
    {
        return data_.template get<std::reference_wrapper<U>>().get();
    }

private:
    /// @brief The wrapped type_list with std::reference_wrapper
    using wrapped_list = typename internals::wrap_with_reference<List>::type;

    /// @brief The wrapped exclude type_list with std::reference_wrapper
    using wrapped_exclude_list = typename internals::wrap_with_reference<type_list<Exclude...>>::type;

    /// @brief The shared resources storage
    typename internals::shared_resources_exclude_list<wrapped_list, wrapped_exclude_list>::type data_;

    /// @brief Allow shared_references to access private members
    template <type_list_concept, typename...>
    friend class shared_references;
};

}  // namespace srs

#endif  // SHARED_RESOURCES_SHARED_RESOURCES_HPP
