/**
 * Copyright (C) 2018 Daniel Kirchner
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#pragma once

#include <crampl/NonTypeList.h>
#include <type_traits>
#include <functional>

namespace crampl {

template<typename T, typename Compare>
struct PointerRange;

namespace detail
{

template<auto M, typename = void>
struct MemberWrapper;

// wrapper around member pointers
template<typename C, typename T, T C::*m>
struct MemberWrapper<m>
{
	typedef C objectType;
	typedef T valueType;

	static const valueType &get(const objectType &obj) noexcept
	{ return obj.*m; }
};

// wrapper around pointers to member functions
template<typename C, typename T, T (C::*m)() const>
struct MemberWrapper<m>
{
	typedef C objectType;
	typedef T valueType;

	static valueType get(const objectType &obj) noexcept
	{ return (obj.*m)(); }
};

// wrapper around integral constants
template<auto M>
struct MemberWrapper<M, std::enable_if_t<std::is_integral_v<decltype(M)>>>
{
	typedef decltype(M) valueType;
	// objectType is undefined

	// can return value for any type of object
	template<typename C>
	static constexpr valueType get(const C &object) noexcept
	{ return M; }
};

// Helper class for obtaining the object type of a list of member specifiers.
// T is a NonTypeList of member specifiers.
template<typename T, typename = void>
struct ObjectTypeHelper;
// Use the objectType member of the MemberWrapper of the first member specifier, if the field is present.
// TODO: Assert that all subsequent member specifiers contain either a matching objectType or no objectType.
template<auto M1, auto... M>
struct ObjectTypeHelper<NonTypeList<M1, M...>, std::void_t<typename MemberWrapper<M1>::objectType>>
{ typedef typename MemberWrapper<M1>::objectType type; };
// Otherwise skip to the next member specifier.
template<typename T, auto M1, auto... M>
struct ObjectTypeHelper<NonTypeList<M1, M...>, T> : ObjectTypeHelper<NonTypeList<M...>, T>
{};

// Obtains the object type of a list of member specifications.
template<auto... M>
using ObjectType = typename ObjectTypeHelper<NonTypeList<M...>>::type;

// Helper class to compare members according to a comparison template.
// compareClass is the template used to instantiate the comparison operators for the members.
// objectType is the base type to be compared.
// Args is a list of MemberWrapper<...> types, that wrap member specifiers.
template<template<typename...> typename CompareTemplate, typename ObjectType, typename... Args>
struct MemberComparator;


// Recursion base case.
template<template<typename...> typename CompareTemplate, typename ObjectType, auto m, typename... Args>
struct MemberComparator<CompareTemplate, ObjectType, MemberWrapper<m>, Args...> {
	bool operator()(const ObjectType &lhs, const ObjectType &rhs) const
	{
		// Compare m.
		CompareTemplate<typename MemberWrapper<m>::valueType> cmp;
		if (cmp(MemberWrapper<m>::get(lhs), MemberWrapper<m>::get(rhs))) return true;
		if (cmp(MemberWrapper<m>::get(rhs), MemberWrapper<m>::get(lhs))) return false;
		// Compare tail.
		return MemberComparator<CompareTemplate, ObjectType, Args...>()(lhs, rhs);
	}
};

// Empty tail case.
template<template<typename...> typename CompareTemplate, typename ObjectType>
struct MemberComparator<CompareTemplate, ObjectType> {
	bool operator()(const ObjectType &lhs, const ObjectType& rhs) const
	{
		return false;
	}
};

}

// Templated member comparator.
// Compares the members specified by M... using the compare template CompareTemplate.
// Deduces the type of the object to be compared from the list of member specifiers.
template<template<typename...> typename CompareTemplate, auto... M>
using MemberComparatorTemplate = detail::MemberComparator<CompareTemplate, detail::ObjectType<M...>, detail::MemberWrapper<M>...>;

// Default member comparison according to std::less
template<auto... M>
using MemberComparator = MemberComparatorTemplate<std::less, M...>;

namespace detail {
// Helper for overloaded member comparison.
template<typename CompareClass, auto... M>
struct MemberComparatorOverloaded {
	template<typename D>
	struct CompareTemplate {
		bool operator()(const D &lhs, const D &rhs) const {
			return CompareClass()(lhs, rhs);
		}
	};
	bool operator()(const detail::ObjectType<M...> &lhs, const detail::ObjectType<M...> &rhs) const {
		return detail::MemberComparator<CompareTemplate, detail::ObjectType<M...>, detail::MemberWrapper<M>...>()(lhs, rhs);
	}
};
}


// Overloaded member comparator.
// Compares the members specified by M... using the operator() overloads of CompareClass.
// Deduces the type of the object to be compared from the list of member specifiers.
template<typename CompareClass, auto... M>
using MemberComparatorOverloaded = detail::MemberComparatorOverloaded<CompareClass, M...>;

namespace detail {

// Helper class for constructing templates from members.
template<template<typename...> typename C, typename T>
struct ConstructTemplateFromMemberHelper;

template<template<typename...> typename C, auto... M>
struct ConstructTemplateFromMemberHelper<C, NonTypeList<M...>>
{
	typedef ObjectType<M...> objectType;
	static constexpr auto get(const ObjectType<M...>& object) noexcept {
		// needs working template deduction
		return C{detail::MemberWrapper<M>::get(object)...};
	}
	typedef decltype(get(std::declval<objectType>())) type;
};

// Register the ConstructTemplateFromMemberHelper with MemberWrapper.
template<template<typename...> typename C, typename T, ConstructTemplateFromMemberHelper<C, T>* c>
struct MemberWrapper<c>
{
	typedef typename ConstructTemplateFromMemberHelper<C, T>::objectType objectType;
	typedef typename ConstructTemplateFromMemberHelper<C, T>::type valueType;
	static constexpr valueType get(const objectType &object) noexcept {
		return c->get(object);

	}
};

// Helper class for constructing objects from members.
template<typename C, typename T>
struct ConstructObjectFromMemberHelper;

template<typename C, auto... M>
struct ConstructObjectFromMemberHelper<C, NonTypeList<M...>>
{

	typedef C type;
	typedef ObjectType<M...> objectType;
	static constexpr type get(const objectType& object) noexcept {
		return type{detail::MemberWrapper<M>::get(object)...};
	}
};

// Register the ConstructObjectFromMemberHelper with MemberWrapper.
template<typename C, typename T, ConstructObjectFromMemberHelper<C, T>* c>
struct MemberWrapper<c>
{
	typedef typename ConstructObjectFromMemberHelper<C, T>::type valueType;
	typedef typename ConstructObjectFromMemberHelper<C, T>::objectType objectType;
	static constexpr valueType get(const objectType &object) noexcept {
		return c->get(object);

	}
};

}

// ConstructFromMembers constructs an object or template of type C from the members specified by M... .
template<typename C, auto... M>
constexpr detail::ConstructObjectFromMemberHelper<C, NonTypeList<M...>>* ConstructFromMembers() { return nullptr; };
template<template<typename...> typename C, auto... M>
constexpr detail::ConstructTemplateFromMemberHelper<C, NonTypeList<M...>>* ConstructFromMembers() { return nullptr; };

template<auto PtrMember, auto SizeMember>
constexpr auto PointerRangeCompare() { return ConstructFromMembers<PointerRange, PtrMember, SizeMember>(); }

template<typename Compare, auto PtrMember, auto SizeMember>
constexpr auto PointerRangeCompare() {
    using T = typename detail::MemberWrapper<PtrMember>::valueType;
    return ConstructFromMembers<PointerRange<T, Compare>, PtrMember, SizeMember>();
}

template<template<typename...> typename Compare, auto PtrMember, auto SizeMember>
constexpr auto PointerRangeCompare() {
    using T = typename detail::MemberWrapper<PtrMember>::valueType;
    using derefValueType = std::decay_t<decltype(*std::declval<T>())>;
    return ConstructFromMembers<PointerRange<T, Compare<derefValueType>>, PtrMember, SizeMember>();
}

}
