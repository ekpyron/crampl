#pragma once

#include <exception>

namespace crampl {

class bad_call : public std::exception {
public:
	virtual ~bad_call() {}
	virtual const char* what() const noexcept {
		return "Bad function call.";
	}
};

namespace detail {

template<typename T, bool IsConst>
class Function;

template<typename R, typename... Args, bool IsConst>
class Function<R(Args...), IsConst> {
	using VoidPtrWithConstness = std::conditional_t<IsConst, const void*, void*>;
	typedef Function<R(Args...), IsConst> selfType;
public:
	Function() = default;
	Function(const Function &) = delete;
	Function(Function&& _rhs) noexcept {
		manager = _rhs.manager; _rhs.manager = noop_manager;
		invoke = _rhs.invoke; _rhs.invoke = bad_invoke;
		move_from(&_rhs);
	}
	Function& operator=(const Function &) = delete;
	Function& operator=(Function && _rhs) noexcept {
		destroy();
		manager = _rhs.manager; _rhs.manager = noop_manager;
		invoke = _rhs.invoke; _rhs.invoke = bad_invoke;
		move_from(&_rhs);
		return *this;
	}
	~Function() {
		destroy();
	}

	template<typename T>
	Function(T&& lambda) noexcept {
		static_assert(std::is_nothrow_move_constructible_v<T>);
		using TPtrWithConstness = std::conditional_t<IsConst, const T*, T*>;
		if constexpr(sizeof(T) <= sizeof(state)) {
			manager = [](selfType* src, selfType* dst) {
				if (dst) {
					// Move
					new (&dst->state) T(std::move(*reinterpret_cast<T*>(&src->state)));
				} else {
					// Destroy
					reinterpret_cast<T*>(&src->state)->~T();
				}
			};
			invoke = [](VoidPtrWithConstness state, Args... args) -> R {
				if constexpr(std::is_same_v<R, void>) {
					(*reinterpret_cast<TPtrWithConstness>(&state))(std::forward<Args>(args)...);
				} else {
					return (*reinterpret_cast<TPtrWithConstness>(&state))(std::forward<Args>(args)...);
				}
			};
			new (&state) T(std::forward<T>(lambda));
		} else {
			manager  = [](selfType *src, selfType *dst) {
				if (dst) {
					// Move
					dst->state = src->state;
				} else {
					// Destroy
					delete reinterpret_cast<T*>(src->state);
				}
			};
			invoke = [](VoidPtrWithConstness state, Args... args) -> R {
				if constexpr(std::is_same_v<R, void>) {
					(*reinterpret_cast<TPtrWithConstness>(state))(std::forward<Args>(args)...);
				} else {
					return (*reinterpret_cast<TPtrWithConstness>(state))(std::forward<Args>(args)...);
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
	VoidPtrWithConstness state;
	R (*invoke)(VoidPtrWithConstness, Args...) = bad_invoke;
	void (*manager)(selfType*, selfType*) = noop_manager;

	void destroy() {
		manager(this, nullptr);
	}
	void move_from(selfType* src) {
		manager(src, this);
	}
	static void noop_manager(selfType*, selfType*) {
	}
	static R bad_invoke(VoidPtrWithConstness, Args...) {
		throw bad_call();
	}

};

}

template<typename T>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)> {
public:
	template<typename... Args2>
	Function(Args2&&... args): impl(std::forward<Args2>(args)...) {}
	Function(const Function&) = delete;
	Function(Function&&) = default;
	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) = default;
	template<typename... Args2>
	R operator()(Args2&&... args) {
		if constexpr (std::is_same_v<R, void>) {
			impl(std::forward<Args2>(args)...);
		} else {
			return impl(std::forward<Args2>(args)...);
		}
	}
private:
	detail::Function<R(Args...), false> impl;
};

template<typename R, typename... Args>
class Function<R(Args...) const> {
public:
	template<typename... Args2>
	Function(Args2&&... args): impl(std::forward<Args2>(args)...) {}
	Function(const Function&) = delete;
	Function(Function&&) = default;
	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) = default;
	template<typename... Args2>
	R operator()(Args2&&... args) const {
		if constexpr (std::is_same_v<R, void>) {
			impl(std::forward<Args2>(args)...);
		} else {
			return impl(std::forward<Args2>(args)...);
		}
	}
private:
	detail::Function<R(Args...), true> impl;
};

}
