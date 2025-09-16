#pragma once
#include "PArena.h"

namespace pstd {

	template<typename S>
	struct Delegate;

	template<typename R, typename... Args>
	struct Delegate<R(Args...)> {
		virtual R operator()(Args... args) = 0;
	};

	template<typename T, typename S>
	struct DelegateImpl;

	template<typename T, typename R, typename... Args>
	struct DelegateImpl<T, R(Args...)> : public Delegate<R(Args...)> {
		DelegateImpl(T p_Function) : function(p_Function) {}

		R operator()(Args... args) override { return function(args...); }

		T function;
	};

	template<typename S, typename T>
	Delegate<S>* makeDelegate(pstd::Arena* pArena, T functor) {
		auto* block{ pstd::alloc<DelegateImpl<T, S>>(pArena) };

		DelegateImpl<T, S>* pDelegateImpl{ new (block)
											   DelegateImpl<T, S>(functor) };

		return static_cast<Delegate<S>*>(pDelegateImpl);
	}

}  // namespace pstd
