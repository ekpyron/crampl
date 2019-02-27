#include <crampl/crampl.h>
#include <catch2/catch.hpp>

class NonCopyableInt {
public:
	NonCopyableInt(int v): value(v) {}
	NonCopyableInt(NonCopyableInt&& v): value(v.value) { v.value = 0; }
	NonCopyableInt(const NonCopyableInt&) = delete;
	NonCopyableInt& operator=(const NonCopyableInt&) = delete;
	NonCopyableInt& operator=(NonCopyableInt&& v) {
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
	CopyMoveCounter(CopyMoveCounter&& _rhs): _copyCount(_rhs._copyCount), _moveCount(_rhs._moveCount + 1) {}
	CopyMoveCounter& operator=(const CopyMoveCounter& _rhs) {
		_copyCount = _rhs._copyCount + 1;
		_moveCount = _rhs._moveCount;
		return *this;
	}
	CopyMoveCounter& operator=(CopyMoveCounter&& _rhs) {
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

TEST_CASE("closure container")
{
	{
		crampl::ClosureContainer<void(int&)> *ptr;
		crampl::TypedClosureContainer x([](int& x) {
			++x;
		});
		ptr = &x;

		int v = 1;
		(*ptr)(v);

		REQUIRE(v == 2);
	}

	{
		std::unique_ptr<crampl::ClosureContainer<int()>> ptr;

		ptr = crampl::make_unique_closure_container([] {
			return 42;
		});

		REQUIRE((*ptr)() == 42);

		ptr = crampl::make_unique_closure_container([ np = NonCopyableInt(21) ] {
			return np.val();
		});

		REQUIRE((*ptr)() == 21);

		NonCopyableInt nci = 23;

		ptr = crampl::make_unique_closure_container([ np = std::move(nci) ] {
			return np.val();
		});

		REQUIRE((*ptr)() == 23);
	}

	{
		std::unique_ptr<crampl::ClosureContainer<int(int)>> ptr;
		ptr = crampl::make_unique_closure_container(std::function<int(int)> ([](auto v) { return v * 3; }));

		REQUIRE((*ptr)(10) == 30);
	}

	{
		CopyMoveCounter c{};
		std::unique_ptr<crampl::ClosureContainer<std::tuple<int, int> (CopyMoveCounter)>> ptr;
		ptr = crampl::make_unique_closure_container([](CopyMoveCounter v) { return std::make_tuple(v.copyCount(), v.moveCount()); });

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE((*ptr)(c) == std::make_tuple(1,1));
	}
	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		std::function<std::tuple<int,int>(CopyMoveCounter)> f;
		f = [](CopyMoveCounter v) { return std::make_tuple(v.copyCount(), v.moveCount()); };

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE(f(c) == std::make_tuple(1,1));
	}
}

TEST_CASE("movable closure container")
{
	{
		crampl::MovableClosureContainer<void(int&)> cc;

		cc = crampl::MovableClosureContainer<void(int&)>([](int& c) {
			++c;
		});

		int v = 1;
		cc(v);
		REQUIRE(v == 2);

		cc = {};
		REQUIRE_THROWS_AS(cc(v), std::runtime_error);
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::MovableClosureContainer<std::tuple<int,int>(CopyMoveCounter)> mcc;
		mcc = [](CopyMoveCounter v) { return std::make_tuple(v.copyCount(), v.moveCount()); };

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE(mcc(c) == std::make_tuple(2, 0));
		REQUIRE(mcc(c) == std::make_tuple(2, 0));
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::MovableClosureContainer<std::tuple<int,int>()> mcc;
		mcc = [c] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		REQUIRE(mcc() == std::make_tuple(1, 2));
		REQUIRE(mcc() == std::make_tuple(1, 2));
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::MovableClosureContainer<std::tuple<int,int>()> mcc;
		mcc = [c = std::move(c)] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};

		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		auto counts = mcc();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
		counts = mcc();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
	}

	{
		CopyMoveCounter c{};
		REQUIRE(c.copyCount() == 0);
		REQUIRE(c.moveCount() == 0);
		crampl::MovableClosureContainer<std::tuple<int,int>()> mcc;
		auto callable = [c = std::move(c)] {
			return std::make_tuple(c.copyCount(), c.moveCount());
		};
		REQUIRE(std::get<0>(callable()) == 0);
		REQUIRE(std::get<1>(callable()) == 1);

		mcc = std::move(callable);

		auto counts = mcc();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 3);
		counts = mcc();
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

		auto counts = crampl::MovableClosureContainer<std::tuple<int,int>()>(std::move(callable))();
		REQUIRE(std::get<0>(counts) == 0);
		REQUIRE(std::get<1>(counts) == 2);
	}

	{
		crampl::MovableClosureContainer<std::tuple<int, int>()> mcc;
		CopyMoveCounter copyCounter{};
		mcc = crampl::MovableClosureContainer<std::tuple<int, int>()>([ &copyCounter ]() {
			return std::make_tuple(copyCounter.copyCount(), copyCounter.moveCount());
		});
		REQUIRE(mcc() == std::make_tuple(0, 0));
		REQUIRE(mcc() == std::make_tuple(0, 0));
	}
}
