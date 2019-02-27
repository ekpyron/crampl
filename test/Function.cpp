#include <crampl/crampl.h>
#include <catch2/catch.hpp>
#include <functional>

class NonCopyableInt {
public:
	NonCopyableInt(int v): value(v) {}
	NonCopyableInt(NonCopyableInt&& v) noexcept : value(v.value) { v.value = 0; }
	NonCopyableInt(const NonCopyableInt&) = delete;
	NonCopyableInt& operator=(const NonCopyableInt&) = delete;
	NonCopyableInt& operator=(NonCopyableInt&& v) noexcept {
		value = v.value;
		v.value = 0;
		return *this;
	}
	int val() const { return value; }
private:
	int value;
};

class CopyMoveCounter {
public:
	CopyMoveCounter() = default;
	CopyMoveCounter(const CopyMoveCounter& _rhs): _copyCount(_rhs._copyCount + 1), _moveCount(_rhs._moveCount) {}
	CopyMoveCounter(CopyMoveCounter&& _rhs) noexcept: _copyCount(_rhs._copyCount), _moveCount(_rhs._moveCount + 1) {}
	CopyMoveCounter& operator=(const CopyMoveCounter& _rhs) {
		_copyCount = _rhs._copyCount + 1;
		_moveCount = _rhs._moveCount;
		return *this;
	}
	CopyMoveCounter& operator=(CopyMoveCounter&& _rhs) noexcept {
		_copyCount = _rhs._copyCount;
		_moveCount = _rhs._moveCount + 1;
		return *this;
	}
	int copyCount() const {
		return _copyCount;
	}
	int moveCount() const {
		return _moveCount;
	}
private:
	int _copyCount = 0;
	int _moveCount = 0;
};

TEST_CASE("function")
{
	{
		crampl::Function<int()> f;
		NonCopyableInt i(42);
		f = [i = std::move(i)] {
			return i.val();
		};
		REQUIRE(f() == 42);
		crampl::Function<int()> g = std::move(f);
		REQUIRE_THROWS_AS(f(), crampl::bad_call);
		REQUIRE(g() == 42);
	}

	{
		crampl::Function<int()> f;
		std::function<int()> sf {[] { return 42; }};
		f = std::move(sf);
		REQUIRE(f() == 42);
	}

	{
		crampl::Function<void(int&)> f;

		f = crampl::Function<void(int&)>([](int& c) {
			++c;
		});

		int v = 1;
		f(v);
		REQUIRE(v == 2);

		f = {};
		REQUIRE_THROWS_AS(f(v), crampl::bad_call);
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::Function<std::tuple<int,int>(CopyMoveCounter)> f;
		f = [](CopyMoveCounter v) { return std::make_tuple(v.copyCount(), v.moveCount()); };

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE(f(c) == std::make_tuple(1, 1));
		REQUIRE(f(c) == std::make_tuple(1, 1));
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::Function<std::tuple<int,int>()> f;
		f = [c] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE(f() == std::make_tuple(1, 2));
		REQUIRE(f() == std::make_tuple(1, 2));
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::Function<std::tuple<int,int>()> f;
		f = [c = std::move(c)] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		auto counts = f();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
		counts = f();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::Function<std::tuple<int,int>()> f;
		auto callable = [c = std::move(c)] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};
		REQUIRE(std::get<0>(callable()) == 0);
		REQUIRE(std::get<1>(callable()) == 1);

		f = std::move(callable);

		auto counts = f();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
		counts = f();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		auto callable = [c = std::move(c)] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};
		REQUIRE(std::get<0>(callable()) == 0);
		REQUIRE(std::get<1>(callable()) == 1);

		auto counts = crampl::Function<std::tuple<int,int>()>(std::move(callable))();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 2);
	}

	{
		crampl::Function<std::tuple<int, int>()> f;
		CopyMoveCounter copyCounter{};
		f = crampl::Function<std::tuple<int, int>()>([ &copyCounter ]() {
			return std::make_tuple(copyCounter.copyCount(), copyCounter.moveCount());
		});
		REQUIRE(f() == std::make_tuple(0, 0));
		REQUIRE(f() == std::make_tuple(0, 0));
	}
}
