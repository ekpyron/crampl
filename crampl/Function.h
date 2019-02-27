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
class Function<R(Args...), IsConst> final {
	using VoidPtrWithConstness = std::conditional_t<IsConst, const void*, void*>;
	typedef Function<R(Args...), IsConst> selfType;
public:
	Function() = default;
	Function(const Function &) = delete;
	template<typename T>
	Function(T&& lambda) noexcept {
		init_from(std::forward<T>(lambda));
	}
	~Function() {
		destroy();
	}

	Function& operator=(const Function&) = delete;
	template<typename T>
	Function& operator=(T&& _rhs) noexcept {
		destroy();
		init_from(std::forward<T>(_rhs));
		return *this;
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

	void init_from(Function&& _rhs) {
		manager = _rhs.manager; _rhs.manager = noop_manager;
		invoke = _rhs.invoke; _rhs.invoke = bad_invoke;
		move_from(&_rhs);
	}

	template<typename T>
	void init_from(T&& lambda) {
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
};

}

template<typename T>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)> {
public:
	Function(const Function&) = delete;
	Function(Function&&) = default;
	template<typename... ConstrArgs>
	Function(ConstrArgs&&... args): impl(std::forward<ConstrArgs>(args)...) {}
	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) = default;
	template<typename T>
	Function& operator=(T&& _lmb) {
		impl = std::forward<T>(_lmb);
		return *this;
	}
	template<typename... CallArgs>
	R operator()(CallArgs&&... args) {
		if constexpr (std::is_same_v<R, void>) {
			impl(std::forward<CallArgs>(args)...);
		} else {
			return impl(std::forward<CallArgs>(args)...);
		}
	}
private:
	detail::Function<R(Args...), false> impl;
};

template<typename R, typename... Args>
class Function<R(Args...) const> {
public:
	Function(const Function&) = delete;
	Function(Function&&) = default;
	template<typename... ConstrArgs>
	Function(ConstrArgs&&... args): impl(std::forward<ConstrArgs>(args)...) {}
	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) = default;
	template<typename T>
	Function& operator=(T&& _lmb) {
		impl = std::forward<T>(_lmb);
		return *this;
	}
	template<typename... CallArgs>
	R operator()(CallArgs&&... args) const {
		if constexpr (std::is_same_v<R, void>) {
			impl(std::forward<CallArgs>(args)...);
		} else {
			return impl(std::forward<CallArgs>(args)...);
		}
	}
private:
	detail::Function<R(Args...), true> impl;
};

}
