#pragma once

#include <memory>
#include <stdexcept>

namespace crampl {

// Variant A

template<typename T>
struct ClosureContainer;

template<typename R, typename... Args>
struct ClosureContainer<R(Args...)> {
	using ReturnValue = R;
	virtual ~ClosureContainer() = default;
	virtual R operator()(Args... args) = 0;
};

namespace detail {

template<typename T, typename OP>
struct TypedClosureContainerHelper;

template<typename T, typename R, typename... Args>
struct TypedClosureContainerHelper<T, R(T::*)(Args...) const>: ClosureContainer<R(Args...)> {
	using type = ClosureContainer<R(Args...)>;
	TypedClosureContainerHelper(T&& c): callable(std::move(c)) {}
	virtual ~TypedClosureContainerHelper() = default;
	R operator()(Args... args) override {
		if constexpr(std::is_same_v<R, void>) {
			callable(std::forward<Args>(args)...);
		} else {
			return callable(std::forward<Args>(args)...);
		}
	}
	T callable;
};

}

template<typename T>
struct TypedClosureContainer: detail::TypedClosureContainerHelper<T, decltype(&T::operator())> {
	using type = typename detail::TypedClosureContainerHelper<T, decltype(&T::operator())>::type;
	TypedClosureContainer(T&& c): detail::TypedClosureContainerHelper<T, decltype(&T::operator())>(std::forward<T>(c)) {
	}
};

template<typename T>
std::unique_ptr<typename TypedClosureContainer<T>::type> make_unique_closure_container(T&& t) {
	return std::make_unique<TypedClosureContainer<T>>(std::forward<T>(t));
}



// Variant B

namespace detail {

template<typename T, bool IsConst>
class MovableClosureContainer;

template<typename R, typename... Args, bool IsConst>
class MovableClosureContainer<R(Args...), IsConst> {
	using VoidPtrWithConstness = std::conditional_t<IsConst, const void*, void*>;
public:
	typedef MovableClosureContainer<R(Args...), IsConst> selfType;
	MovableClosureContainer() = default;
	MovableClosureContainer(const MovableClosureContainer &) = delete;
	MovableClosureContainer(MovableClosureContainer&& _rhs) noexcept {
		manager = _rhs.manager; _rhs.manager = noop_manager;
		invoke = _rhs.invoke; _rhs.invoke = bad_invoke;
		manager(Operation::Move, &_rhs, this);
	}
	MovableClosureContainer& operator=(const MovableClosureContainer &) = delete;
	MovableClosureContainer& operator=(MovableClosureContainer && _rhs) noexcept {
		manager(Operation::Destroy, this, nullptr);
		manager = _rhs.manager; _rhs.manager = noop_manager;
		invoke = _rhs.invoke; _rhs.invoke = bad_invoke;
		manager(Operation::Move, &_rhs, this);
		return *this;
	}
	~MovableClosureContainer() {
		manager(Operation::Destroy, this, nullptr);
	}

	template<typename T>
	MovableClosureContainer(T&& lambda) noexcept(std::is_nothrow_move_constructible_v<T>) {
		using TPtrWithConstness = std::conditional_t<IsConst, const T*, T*>;
		if constexpr(sizeof(T) <= sizeof(state)) {
			manager = [](Operation op, selfType* src, selfType* dst) {
				switch (op) {
					case Operation::Destroy:
						reinterpret_cast<T*>(&src->state)->~T();
						break;
					case Operation::Move:
						new (&dst->state) T(std::move(*reinterpret_cast<T*>(&src->state)));
						break;
				}
			};
			invoke = [](VoidPtrWithConstness state, Args... args) -> R {
				if constexpr(std::is_same_v<R, void>) {
					(*reinterpret_cast<TPtrWithConstness>(&state))(args...);
				} else {
					return (*reinterpret_cast<TPtrWithConstness>(&state))(args...);
				}
			};
			new (&state) T(std::forward<T>(lambda));
		} else {
			manager  = [](Operation op, selfType *src, selfType *dst) {
				switch (op) {
					case Operation::Destroy:
						delete reinterpret_cast<T*>(src->state);
						break;
					case Operation::Move:
						dst->state = src->state;
						break;
				}
			};
			invoke = [](VoidPtrWithConstness state, Args... args) -> R {
				if constexpr(std::is_same_v<R, void>) {
					(*reinterpret_cast<TPtrWithConstness>(state))(args...);
				} else {
					return (*reinterpret_cast<TPtrWithConstness>(state))(args...);
				}
			};
			state = new T(std::forward<T>(lambda));
		}
	}
	template<typename... Args2>
	R operator()(Args2&&... args) const {
		return invoke(state, std::forward<Args2>(args)...);
	}
	template<typename... Args2>
	R operator()(Args2&&... args) {
		return invoke(state, std::forward<Args2>(args)...);
	}

private:
	void* state;
	R (*invoke)(VoidPtrWithConstness, Args...) = bad_invoke;

	enum class Operation {
		Move,
		Destroy
	};

	void (*manager)(Operation op, selfType*, selfType*) = noop_manager;
	static void noop_manager(Operation op, selfType*, selfType*) {
	}
	static R bad_invoke(VoidPtrWithConstness, Args...) {
		throw std::runtime_error("Bad movable closure container call.");
	}

};

}

template<typename T>
class MovableClosureContainer;

template<typename R, typename... Args>
class MovableClosureContainer<R(Args...)>: public detail::MovableClosureContainer<R(Args...), false> {
public:
	template<typename... Args2>
	MovableClosureContainer(Args2&&... args): detail::MovableClosureContainer<R(Args...), false>(std::forward<Args2>(args)...) {}
};

template<typename R, typename... Args>
class MovableClosureContainer<R(Args...) const>: public detail::MovableClosureContainer<R(Args...), true> {
public:
	template<typename... Args2>
	MovableClosureContainer(Args2&&... args): detail::MovableClosureContainer<R(Args...), true>(std::forward<Args2>(args)...) {}
};

}