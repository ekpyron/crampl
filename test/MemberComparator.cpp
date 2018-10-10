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

#include <crampl/crampl.h>
#include <catch2/catch.hpp>
#include <algorithm>

TEST_CASE("Single integer struct comparison.", "[single_int]")
{
	struct S { int a; };
	crampl::MemberComparator<&S::a> cmp;
	S lhs, rhs;
	for (lhs.a = -1; lhs.a < 2; ++lhs.a)
		for (rhs.a = -1; rhs.a < 2; ++rhs.a)
				REQUIRE(cmp(lhs, rhs) == (lhs.a < rhs.a));
}

TEST_CASE("Two integer struct full comparison.", "[two_int]")
{
	struct S { int a; int b; };
	crampl::MemberComparator<&S::a, &S::b> cmp;
	auto reference = [](S const& lhs, S const& rhs) -> bool {
		if (lhs.a < rhs.a) return true;
		if (rhs.a < lhs.a) return false;
		return lhs.b < rhs.b;
	};
	S lhs, rhs;
	for (lhs.a = -1; lhs.a < 2; ++lhs.a)
		for (lhs.b = -1; lhs.b < 2; ++lhs.b)
			for (rhs.a = -1; rhs.a < 2; ++rhs.a)
				for (rhs.b = -1; rhs.b < 2; ++rhs.b)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}

TEST_CASE("Two integer struct first member comparison.", "[two_int_first]")
{
	struct S { int a; int b; };
	crampl::MemberComparator<&S::a> cmp;
	auto reference = [](S const& lhs, S const& rhs) -> bool {
		return lhs.a < rhs.a;
	};
	S lhs, rhs;
	for (lhs.a = -1; lhs.a < 2; ++lhs.a)
		for (lhs.b = -1; lhs.b < 2; ++lhs.b)
			for (rhs.a = -1; rhs.a < 2; ++rhs.a)
				for (rhs.b = -1; rhs.b < 2; ++rhs.b)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}

TEST_CASE("Two integer struct second member comparison.", "[two_int_second]")
{
	struct S { int a; int b; };
	crampl::MemberComparator<&S::b> cmp;
	auto reference = [](S const& lhs, S const& rhs) -> bool {
		return lhs.b < rhs.b;
	};
	S lhs, rhs;
	for (lhs.a = -1; lhs.a < 2; ++lhs.a)
		for (lhs.b = -1; lhs.b < 2; ++lhs.b)
			for (rhs.a = -1; rhs.a < 2; ++rhs.a)
				for (rhs.b = -1; rhs.b < 2; ++rhs.b)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}

namespace {
template<typename T>
struct CustomCompare;
template<>
struct CustomCompare<int>
{
	bool operator()(int lhs, int rhs) const {
		return (lhs & 1) < (rhs & 1);
	}
};
template<>
struct CustomCompare<float>
{
	bool operator()(float lhs, float rhs) const {
		return lhs > rhs;
	}
};
}

TEST_CASE("Comparison with custom compare template.", "[custom_compare_template]")
{
	struct S { int a; float b; };
	crampl::MemberComparatorTemplate<CustomCompare, &S::a, &S::b> cmp;
	auto reference = [](const S &lhs, const S &rhs) -> bool {
		if (CustomCompare<int>()(lhs.a, rhs.a)) return true;
		if (CustomCompare<int>()(rhs.a, lhs.a)) return false;
		return CustomCompare<float>()(lhs.b, rhs.b);

	};
	S lhs, rhs;
	for (lhs.a = -2; lhs.a <= 2; ++lhs.a)
		for (rhs.a = -2; rhs.a <= 2; ++rhs.a)
			for (lhs.b = -1.0f; lhs.b <= 1.0f; lhs.b += 0.25f)
				for (rhs.b = -1.0f; rhs.b <= 1.0f; rhs.b += 0.25f)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}

template<auto... M>
using MyCompare = crampl::MemberComparatorTemplate<CustomCompare, M...>;

TEST_CASE("Comparison with custom compare template alias.", "[custom_compare_template_alias]")
{
	struct S { int a; float b; };
	MyCompare<&S::a, &S::b> cmp;
	auto reference = [](const S &lhs, const S &rhs) -> bool {
		if (CustomCompare<int>()(lhs.a, rhs.a)) return true;
		if (CustomCompare<int>()(rhs.a, lhs.a)) return false;
		return CustomCompare<float>()(lhs.b, rhs.b);

	};
	S lhs, rhs;
	for (lhs.a = -2; lhs.a <= 2; ++lhs.a)
		for (rhs.a = -2; rhs.a <= 2; ++rhs.a)
			for (lhs.b = -1.0f; lhs.b <= 1.0f; lhs.b += 0.25f)
				for (rhs.b = -1.0f; rhs.b <= 1.0f; rhs.b += 0.25f)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}


struct OverloadedCompare {
	bool operator()(const int& lhs, const int& rhs) const {
		return (lhs & 1) < (rhs & 1);
	}
	bool operator()(const float& lhs, const float& rhs) const {
		return lhs > rhs;
	}
};

TEST_CASE("Comparison with custom compare overloads.", "[custom_compare_overload]")
{
	struct S { int a; float b; };
	crampl::MemberComparatorOverloaded<OverloadedCompare, &S::a, &S::b> cmp;
	auto reference = [](const S &lhs, const S &rhs) -> bool {
		if (OverloadedCompare()(lhs.a, rhs.a)) return true;
		if (OverloadedCompare()(rhs.a, lhs.a)) return false;
		return OverloadedCompare()(lhs.b, rhs.b);

	};
	S lhs, rhs;
	for (lhs.a = -2; lhs.a <= 2; ++lhs.a)
		for (rhs.a = -2; rhs.a <= 2; ++rhs.a)
			for (lhs.b = -1.0f; lhs.b <= 1.0f; lhs.b += 0.25f)
				for (rhs.b = -1.0f; rhs.b <= 1.0f; rhs.b += 0.25f)
					REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
}

template<typename PointerType, typename SizeType>
struct PointerAndSize {
	PointerType ptr;
	SizeType size;
	bool operator<(const PointerAndSize &rhs) const {
		return std::lexicographical_compare(ptr, ptr ? ptr + size : nullptr, rhs.ptr, rhs.ptr ? rhs.ptr + rhs.size : nullptr);
	}
};

template<typename PointerType, typename SizeType>
PointerAndSize(PointerType c, SizeType s) -> PointerAndSize<std::decay_t<PointerType>, std::decay_t<SizeType>>;

template<typename S, typename CompareType>
void test_complex_member(std::function<bool(const S&, const S&)> reference)
{
	CompareType cmp;
	// both nullptr
	{
		S lhs{nullptr, 0};
		S rhs{nullptr, 0};
		for (lhs.size = 0; lhs.size < 2; ++lhs.size)
			for (rhs.size = 0; rhs.size < 2; ++rhs.size)
				REQUIRE(cmp(lhs, rhs) == reference(lhs, rhs));
	}
	// right nullptr
	{
		std::array<int, 2> data;
		S lhs{data.data(), data.size()};
		S rhs{nullptr, 0};
		for (data[0] = -1; data[0] < 2; ++data[0])
			for (data[1] = -1; data[1] < 2; ++data[1])
				REQUIRE(cmp(lhs, rhs) == reference (lhs, rhs));
	}
	// left nullptr
	{
		std::array<int, 2> data;
		S lhs{nullptr, 0};
		S rhs{data.data(), data.size()};
		for (data[0] = -1; data[0] < 2; ++data[0])
			for (data[1] = -1; data[1] < 2; ++data[1])
				REQUIRE(cmp(lhs, rhs) == reference (lhs, rhs));
	}
	// neither nullptr
	{
		std::array<int, 2> data_lhs;
		std::array<int, 2> data_rhs;
		S lhs{data_lhs.data(), data_lhs.size()};
		S rhs{data_rhs.data(), data_rhs.size()};
		for (data_lhs[0] = -1; data_lhs[0] < 2; ++data_lhs[0])
			for (data_lhs[1] = -1; data_lhs[1] < 2; ++data_lhs[1])
				for (data_rhs[0] = -1; data_rhs[0] < 2; ++data_rhs[0])
					for (data_rhs[1] = -1; data_rhs[1] < 2; ++data_rhs[1])
						REQUIRE(cmp(lhs, rhs) == reference (lhs, rhs));
	}
}

TEST_CASE("Complex member PointerAndSize comparison.", "[complex_pointer_and_size]")
{
	struct S
	{
		int *ptr;
		int size;
	};
	typedef crampl::MemberComparator<crampl::ConstructFromMembers<PointerAndSize, &S::ptr, &S::size>()> CompareType;
	test_complex_member<S, CompareType>([](const S &lhs, const S &rhs) -> bool {
		return PointerAndSize { lhs.ptr, lhs. size } < PointerAndSize { rhs.ptr, rhs.size };
	});
}

TEST_CASE("Complex member PointerRange comparison.", "[complex_pointer_range]")
{
	struct S
	{
		int *ptr;
		int size;
	};
	typedef crampl::MemberComparator<crampl::ConstructFromMembers<crampl::PointerRange, &S::ptr, &S::size>()> CompareType;
	test_complex_member<S, CompareType>([](const S &lhs, const S &rhs) -> bool {
		return crampl::PointerRange { lhs.ptr, lhs. size } < crampl::PointerRange { rhs.ptr, rhs.size };
	});
}

template<auto PtrMember, auto SizeMember>
constexpr auto MyPointerRangeCompare() { return crampl::ConstructFromMembers<crampl::PointerRange, PtrMember, SizeMember>(); }

TEST_CASE("Complex member PointerRange comparison alias.", "[complex_pointer_range_alias]")
{
	struct S
	{
		int *ptr;
		int size;
	};
	typedef crampl::MemberComparator<MyPointerRangeCompare<&S::ptr, &S::size>()> CompareType;
	test_complex_member<S, CompareType>([](const S &lhs, const S &rhs) -> bool {
		return crampl::PointerRange { lhs.ptr, lhs. size } < crampl::PointerRange { rhs.ptr, rhs.size };
	});
}

TEST_CASE("Getter function comparison.", "[getter_function]")
{
	struct S { int get() const { return a; } int a; };
	crampl::MemberComparator<&S::get> cmp;
	S lhs, rhs;
	for (lhs.a = -1; lhs.a < 2; ++lhs.a)
		for (rhs.a = -1; rhs.a < 2; ++rhs.a)
			REQUIRE(cmp(lhs, rhs) == (lhs.a < rhs.a));
}
